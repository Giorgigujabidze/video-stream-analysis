#ifndef STREAM_ANALYSIS_STREAM_ANALYZER_HPP
#define STREAM_ANALYSIS_STREAM_ANALYZER_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "metrics.hpp"

void analyzeVideoStream(cv::VideoCapture &cap, const Config &config, const std::vector<ColorRange> &colorRanges,
                        Metrics &metrics, std::vector<double> &buffer1);

#endif // STREAM_ANALYSIS_STREAM_ANALYZER_HPP