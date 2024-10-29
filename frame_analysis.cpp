#include <opencv2/opencv.hpp>
#include "config.hpp"



bool detectColouredStripes(const cv::Mat &frame, const std::vector<ColorRange> &colorRanges, const double &threshold1,
                           const double &threshold2) {
    cv::Mat hsvFrame;
    cv::cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
    cv::Mat combinedMask = cv::Mat::zeros(frame.size(), CV_8U);
    std::vector<double> colorDistributions = {};

    for (const auto &range: colorRanges) {
        cv::Mat mask;
        cv::inRange(hsvFrame, range.lower, range.upper, mask);
        combinedMask |= mask;
        colorDistributions.push_back((mean(mask)[0] / 255) * 100);
    }

    cv::Scalar meanVal, stdDevVal;
    cv::meanStdDev(colorDistributions, meanVal, stdDevVal);

    const double scalingFactor = 100.0 / colorRanges.size();
    const double colouredStripesProbability = (meanVal[0] / scalingFactor) * 100;

    if (colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2) {
        cv::imshow("coloured stripes", combinedMask);
    }
    return colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2;
}

bool detectBlackFrame(const cv::Mat &frame, const double &threshold) {
    return mean(frame)[0] < threshold;
}

bool detectStaticFrame(const cv::Mat &frame, const cv::Mat &prevFrame, const double &threshold,
                       std::vector<double> &buffer,
                       const int maxBufferSize) {
    cv::Mat diff;
    cv::absdiff(frame, prevFrame, diff);
    double bufferAverage = 0;
    const double avgDifference = mean(diff)[0];
    if (buffer.size() < maxBufferSize) {
        buffer.push_back(avgDifference);
        return false;
    }
    bufferAverage = cv::mean(buffer)[0];
    buffer.clear();
    return bufferAverage < threshold;
}