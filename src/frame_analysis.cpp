#include "frame_analysis.hpp"

#include <opencv2/opencv.hpp>
#include "config.hpp"


bool detectColouredStripes(const ColouredStripesData &data) {
    cv::Mat hsvFrame;
    cvtColor(data.frame, hsvFrame, cv::COLOR_BGR2HSV);
    cv::Mat combinedMask = cv::Mat::zeros(data.frame.size(), CV_8U);
    std::vector<double> colorDistributions = {};

    for (const auto &range: data.colorRanges) {
        cv::Mat mask;
        inRange(hsvFrame, range.lower, range.upper, mask);
        combinedMask |= mask;
        colorDistributions.push_back((mean(mask)[0] / 255) * 100);
    }

    cv::Scalar meanVal, stdDevVal;
    meanStdDev(colorDistributions, meanVal, stdDevVal);

    const double scalingFactor = 100.0 / data.colorRanges.size();
    const double colouredStripesProbability = (meanVal[0] / scalingFactor) * 100;

    return colouredStripesProbability > data.threshold1 && stdDevVal[0] < data.threshold2;
}

bool detectBlackFrame(const BlackFrameData &data) {
    return mean(data.frame)[0] < data.threshold;
}

bool detectStaticFrame(const StaticFrameData &data) {
    cv::Mat diff;
    absdiff(data.frame, data.prevFrame, diff);

    double bufferAverage = 0;
    const double avgDifference = mean(diff)[0];

    if (data.buffer.size() < data.maxBufferSize) {
        data.buffer.push_back(avgDifference);
        return false;
    }
    bufferAverage = cv::mean(data.buffer)[0];
    data.buffer.clear();
    return bufferAverage < data.threshold;
}
