#ifndef STREAM_ANALYSIS_STREAM_ANALYZER_HPP
#define STREAM_ANALYSIS_STREAM_ANALYZER_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"

int openVideoStream(cv::VideoCapture &cap, Config &config);


void *analyzeVideoStream(void *threadArgs);

#endif // STREAM_ANALYSIS_STREAM_ANALYZER_HPP
