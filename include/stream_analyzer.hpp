#ifndef STREAM_ANALYSIS_STREAM_ANALYZER_HPP
#define STREAM_ANALYSIS_STREAM_ANALYZER_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "metrics.hpp"
#include "threading.hpp"

struct AnalysisData {
    const cv::Mat &frame;
    const cv::Mat &prevFrame;
    const cv::Mat &downscaledFrame;
    std::vector<double> meanBuffer;
    const ThreadArguments *args;
    Metrics &metrics;
};

void *analyzeVideoStream(void *threadArgs);

void analyzeFrames(AnalysisData &data);

#endif // STREAM_ANALYSIS_STREAM_ANALYZER_HPP
