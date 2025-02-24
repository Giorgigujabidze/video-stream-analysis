#include "helpers.hpp"

#include <fstream>
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "capture.hpp"
#include "config.hpp"
#include "ffmpeg_capture.hpp"
#include "metrics.hpp"
#include "stream_analyzer.hpp"

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
    std::ofstream outfile;
    if (append) {
        outfile.open(filename, std::ios_base::app);
    } else {
        outfile.open(filename, std::ios_base::trunc);
    }
    outfile << content;
}

int openVideoStream(Capture &cap, const std::string &url, const Config &config) {
    if (cap.openStream(url, config) < 0) {
        log(config, "failed to open video stream", ERROR);
        filePutContents("../failed_streams/failed_streams.txt", url + "\n", true);
        cap.release();
        return -1;
    }
    return 0;
}

void preprocessFrame(const cv::Mat &frame, cv::Mat &downscaledFrame, cv::Mat &grayFrame) {
    resize(frame, downscaledFrame, cv::Size(600, 400), 0, 0, cv::INTER_LINEAR);
    cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);
}

int saveAndReset(const std::string &filename, Metrics &metrics, int &frameCount, std::vector<double> &meanBuffer,
                 std::chrono::time_point<std::chrono::system_clock> &start) {
    if (writeResultsToJson(filename, metrics) < 0) {
        return -1;
    }
    frameCount = 0;
    meanBuffer.erase(meanBuffer.begin(), meanBuffer.end());
    metrics = Metrics{};
    start = std::chrono::high_resolution_clock::now();
    return 0;
}

int reconnect(const std::string &filename, Metrics &metrics, Capture &cap, const Config &config,
              const std::string &url) {
    metrics.no_input_stream = true;
    log(config, "connection lost. attempting to reconnect to: " + url, ERROR);
    cap.release();
    if (writeResultsToJson(filename, metrics) < 0) {
        log(config, "failed to reconnect to " + url, ERROR);
        return -1;
    }

    while (true) {
        if (openVideoStream(cap, url, config) >= 0) {
            log(config, "reconnected to " + url, INFO);
            metrics = Metrics{};
            break;
        }
        log(config, "failed to reconnect to " + url, ERROR);
        sleep(5);
    }
    return 0;
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
    switch (level) {
        case INFO:
            std::cout << to_string(level) << ": " << message << '\n';
            break;
        case WARNING:
        case ERROR:
            std::cerr << to_string(level) << ": " << message << '\n';
    }
    std::cout << "--------------------------------------------------" << std::endl;
}
