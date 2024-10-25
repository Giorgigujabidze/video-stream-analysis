#ifndef STREAM_ANALYSIS_FRAME_ANALYSIS_HPP
#define STREAM_ANALYSIS_FRAME_ANALYSIS_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "metrics.hpp"

bool detectBlackFrame(const cv::Mat &frame, const double &threshold);

bool detectStaticFrame(const cv::Mat &frame, const cv::Mat &prevFrame,
                       const double &threshold, std::vector<double> &buffer,
                       int maxBufferSize);

bool detectColouredStripes(const cv::Mat &frame,
                           const std::vector<ColorRange> &colorRanges,
                           const double &threshold1, const double &threshold2);

double getSmoothedMSSIM(const std::vector<double> &mssimBuffer, int windowSize = 5);

double calculateMSSIM(const cv::Mat &frame, const cv::Mat &prevFrame);

double calculateMultiscaleMSSIM(const cv::Mat &frame, const cv::Mat &prevFrame);

#endif