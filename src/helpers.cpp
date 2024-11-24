#include "helpers.hpp"

#include <fstream>
#include <opencv2/videoio.hpp>
#include <opencv2/core/utils/logger.hpp>

#include "config.hpp"
#include "metrics.hpp"
#include "stream_analyzer.hpp"

void programSetup() {
    setenv("OPENCV_FFMPEG_IS_THREAD_SAFE", "1", 0);
    setenv("OPENCV_FFMPEG_LOGLEVEL", "-8", 0);
    setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    filePutContents("../failed_streams/failed_streams.txt", "", false);
}

void getHelp(const std::string &name) {
    std::cout << "usages: " << std::endl
            << name << std::endl
            << name << " <-c> <data_file>" << std::endl;
}

int startConfigMaker(const std::string &filename) {
    std::vector<StreamData> streamDataVector;
    readDataFromFile(filename, streamDataVector);

    if (configMaker("../sample_config/config.json", streamDataVector) < 0) {
        std::cerr << "failed to read sample config file\n";
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

int openVideoStream(cv::VideoCapture &cap, Config &config) {
    if (!cap.open(config.url, config.api_backend, {
                      cv::CAP_PROP_HW_ACCELERATION, config.hardware_acceleration, cv::CAP_PROP_OPEN_TIMEOUT_MSEC, 30000
                  })) {
        std::cerr << "failed to open video stream\n";
        filePutContents("../failed_streams/failed_streams.txt", config.url + "\n", true);
        cap.release();
        return -1;
    }
    return 0;
}

void preprocessFrame(cv::Mat &frame, cv::Mat &downscaledFrame, cv::Mat &grayFrame) {
    resize(frame, downscaledFrame, cv::Size(600, 400), 0, 0, cv::INTER_LINEAR);
    cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);
    cv::cvtColor(frame, frame, cv::COLOR_BGR2RGB);
}

void saveAndReset(const std::string &filename, Metrics &metrics, int &frameCount, std::vector<double> &meanBuffer,
                  std::chrono::time_point<std::chrono::system_clock> &start) {
    writeResultsToJson(filename, metrics);
    frameCount = 0;
    meanBuffer.erase(meanBuffer.begin(), meanBuffer.end());
    metrics = Metrics{};
    start = std::chrono::high_resolution_clock::now();
}

void reconnect(const std::string &filename, Metrics &metrics, cv::VideoCapture &cap, Config &config) {
    metrics.no_input_stream = true;
    std::cerr << "connection lost. attempting to reconnect to: " << config.url << std::endl;
    cap.release();
    writeResultsToJson(filename, metrics);

    while (true) {
        if (openVideoStream(cap, config) >= 0) {
            std::cout << "reconnected\n";
            metrics = Metrics{};
            break;
        }

        std::cerr << "failed to reconnect\n";
        sleep(5);
    }
}
