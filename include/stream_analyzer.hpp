#ifndef STREAM_ANALYSIS_STREAM_ANALYZER_HPP
#define STREAM_ANALYSIS_STREAM_ANALYZER_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "metrics.hpp"
#include "threading.hpp"

int openVideoStream(cv::VideoCapture &cap, Config &config);

void *analyzeVideoStream(void *threadArgs);

void analyzeFrames(const cv::Mat &grayFrame, const cv::Mat &prevGrayFrame, const cv::Mat &downscaledFrame,
                   std::vector<double> &meanBuffer, Metrics &metrics, const ThreadArguments *args);

#endif // STREAM_ANALYSIS_STREAM_ANALYZER_HPP
