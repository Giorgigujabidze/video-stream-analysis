#include "helpers.hpp"

#include <atomic>
#include <fstream>
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "capture.hpp"
#include "config.hpp"
#include "ffmpeg_capture.hpp"
#include "metrics.hpp"
#include "stream_analyzer.hpp"

static pthread_mutex_t fileMutex = PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t logMutex = PTHREAD_MUTEX_INITIALIZER;

void programSetup() {
    setenv("OPENCV_FFMPEG_IS_THREAD_SAFE", "1", 0);
    setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    av_log_set_level(AV_LOG_QUIET);
    filePutContents("../failed_streams/failed_streams.txt", "", false);
}

void getHelp(const std::string &name) {
    std::cout << "usages: " << std::endl
            << name << std::endl
            << name << "<-n> <count>" << std::endl
            << name << " <-c> <data_file>" << std::endl;
}

int startStreamsFileMaker(const std::string &filename) {
    std::vector<StreamData> streamDataVector;

    if (readDataFromFile(filename, streamDataVector) < 0) {
        return -1;
    }

    if (streamsJsonMaker(streamDataVector) < 0) {
        std::cerr << "failed to read sample streams file\n";
        return -1;
    }

    std::cout << "configs where successfully generated\n";
    return 0;
}

void filePutContents(const std::string &filename, const std::string &content, const bool append = false) {
    pthread_mutex_lock(&fileMutex);
    std::ofstream outfile;
    if (append) {
        outfile.open(filename, std::ios_base::app);
    } else {
        outfile.open(filename, std::ios_base::trunc);
    }
    outfile << content;
    pthread_mutex_unlock(&fileMutex);
}

int openVideoStream(Capture &cap, const std::string &url, const Config &config) {
    if (cap.openStream(url, config) < 0) {
        log(config, "failed to open video stream", ERROR);
        filePutContents("../failed_streams/failed_streams.txt", url + "\n", true);
        cap.releaseStream();
        return -1;
    }
    return 0;
}

void preprocessFrame(const cv::Mat &frame, cv::Mat &downscaledFrame, cv::Mat &grayFrame) {
    resize(frame, downscaledFrame, cv::Size(600, 400), 0, 0, cv::INTER_LINEAR);
    cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);
}

int saveAndReset(ResetData &resetData, SaveData &saveData) {
    if (writeResultsToJson(saveData.filename, saveData.metrics) < 0) {
        return -1;
    }
    resetData.reset();
    return 0;
}

int reconnect(ReconnectData &data) {
    data.metrics.no_input_stream = true;
    log(data.config, "connection lost. attempting to reconnect to: " + data.url, ERROR);

    data.cap.releaseStream();

    if (writeResultsToJson(data.filename, data.metrics) < 0) {
        log(data.config, "failed to save data for " + data.url, ERROR);
        return -1;
    }

    int sleepTime = 10;
    while (!data.stopFlag->load()) {
        sleepTime = std::min(sleepTime++, 120);
        sleep(sleepTime);
        if (openVideoStream(data.cap, data.url, data.config) >= 0) {
            log(data.config, "reconnected to " + data.url, INFO);
            data.metrics = Metrics{};
            break;
        }
        log(data.config, "failed to reconnect to " + data.url, ERROR);
        sleep(sleepTime / 2);
    }
    return 0;
}

bool handleDecodeResponse(const decode_status_t resp, Metrics &metrics, const ThreadArguments *args) {
    switch (resp) {
        case DECODE_OK:
            break;
        case DECODE_ERROR:
            metrics.corrupt_frame_count++;
            log(args->config, "retrieveFrame error", ERROR);
            return false;
        case NOT_ENOUGH_DATA:
        case NON_VIDEO_PACKET:
            return false;
    }
    return true;
}

bool onIntervalElapsed(ResetData &resetData, SaveData &saveData) {
    if (saveAndReset(resetData, saveData) < 0) {
        return false;
    }

    if (saveData.arguments->config.save_last_frame) {
        if (!imwrite(saveData.imageName, saveData.image)) {
            log(saveData.arguments->config, saveData.imageName + " save failed " + saveData.arguments->stream.url,
                ERROR);
            return false;
        }
    }
    return true;
}

void validateProgramConfig(Config &config) {
    if (config.api_backend != 1900 /*&& config.api_backend != 1800*/) {
        config.api_backend = 1900;
        log(config, "invalid api backend defaulting to ffmpeg", WARNING);
    }
    if (config.hardware_acceleration > 4) {
        config.hardware_acceleration = 0;
        log(config, "invalid acceleration value defaulting to no hardware acceleration", WARNING);
    }
    if (config.key_frames_only && config.size_parameters.max_mean_buffer_size != 1) {
        config.size_parameters.max_mean_buffer_size = 1;
        log(config, "invalid size parameter value defaulting to 1", WARNING);
    }
    if (config.thresholds.black_frame_threshold < 15) {
        log(config, "invalid black frame threshold value", WARNING);
    } else if (config.thresholds.black_frame_threshold > 15) {
        log(config, "black frame threshold value too high, defaulting to 15", WARNING);
        config.thresholds.black_frame_threshold = 15;
    }

    if (config.thresholds.coloured_stripes_max_deviation > 10) {
        log(config, "deviation threshold value might be too high, defaulting to 10", WARNING);
        config.thresholds.coloured_stripes_max_deviation = 10;
    }

    if (config.thresholds.static_frame_threshold < 0.000001) {
        log(config, "static_frame_threshold value might be too low", WARNING);
    }
}

void log(const Config &config, const std::string &message, const LogLevel level) {
    if (!config.output_to_console) {
        return;
    }
    pthread_mutex_lock(&logMutex);
    switch (level) {
        case INFO:
            std::cout << to_string(level) << ": " << message << std::endl;
            std::cout << "--------------------------------------------------" << std::endl;

            break;
        case WARNING:
        case ERROR:
            std::cerr << to_string(level) << ": " << message << std::endl;
            std::cerr << "--------------------------------------------------" << std::endl;
    }
    pthread_mutex_unlock(&logMutex);
}
