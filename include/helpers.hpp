//
// Created by gio on 11/22/24.
//

#ifndef HELPERS_HPP
#define HELPERS_HPP
#include <string>
#include <chrono>
#include <opencv2/videoio.hpp>

#include "config.hpp"
#include "metrics.hpp"

void programSetup();

void getHelp(const std::string &name);

int startConfigMaker(const std::string &filename);

void filePutContents(const std::string &filename, const std::string &content, bool append);

int openVideoStream(cv::VideoCapture &cap, Config &config);

void preprocessFrame(cv::Mat &frame, cv::Mat &downscaledFrame, cv::Mat &grayFrame);

void reconnect(const std::string &filename, Metrics &metrics, cv::VideoCapture &cap, Config &config);

void saveAndReset(const std::string &filename, Metrics &metrics, int &frameCount, std::vector<double> &meanBuffer,
                  std::chrono::time_point<std::chrono::system_clock> &start);


#endif //HELPERS_HPP