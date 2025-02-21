//
// Created by gio on 11/22/24.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <string>
#include <chrono>
#include <opencv2/videoio.hpp>

#include "capture.hpp"
#include "config.hpp"
#include "ffmpeg_capture.hpp"
#include "metrics.hpp"

enum LogLevel {
    INFO,
    WARNING,
    ERROR
};

inline const char *to_string(LogLevel e) {
    switch (e) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case ERROR: return "ERROR";
        default: return "unknown";
    }
}

void programSetup();

void getHelp(const std::string &name);

int startStreamsFileMaker(const std::string &filename);

void filePutContents(const std::string &filename, const std::string &content, bool append);

int openVideoStream(Capture &cap, const std::string &url, const Config &config);

void preprocessFrame(const cv::Mat &frame, cv::Mat &downscaledFrame, cv::Mat &grayFrame);

int reconnect(const std::string &filename, Metrics &metrics, Capture &cap, const Config &config,
              const std::string &url);

int saveAndReset(const std::string &filename, Metrics &metrics, int &frameCount, std::vector<double> &meanBuffer,
                 std::chrono::time_point<std::chrono::system_clock> &start);

void validateProgramConfig(Config &config);

void log(const Config& config, const std::string& message, LogLevel level);

#endif //HELPERS_HPP
