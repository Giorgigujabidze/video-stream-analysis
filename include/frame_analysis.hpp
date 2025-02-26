#ifndef STREAM_ANALYSIS_FRAME_ANALYSIS_HPP
#define STREAM_ANALYSIS_FRAME_ANALYSIS_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"

struct BlackFrameData {
    const cv::Mat &frame;
    const double &threshold;
};


struct StaticFrameData {
    const cv::Mat &frame;
    const cv::Mat &prevFrame;
    const double &threshold;
    std::vector<double> &buffer;
    const int maxBufferSize;
};

struct ColouredStripesData{
    const cv::Mat &frame;
    const std::vector<ColorRange> &colorRanges;
    const double &threshold1;
    const double &threshold2;
};

bool detectBlackFrame(const BlackFrameData &data);

bool detectStaticFrame(const StaticFrameData &data);

bool detectColouredStripes(const ColouredStripesData &data);


#endif