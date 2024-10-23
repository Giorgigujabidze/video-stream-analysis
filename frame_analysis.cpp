#include <opencv2/opencv.hpp>
#include "config.hpp"
#include "metrics.hpp"



bool isKeyFrame(const cv::Mat &frame, const cv::Mat &prevFrame, double &threshold, const float **histRange, const int &histSize,
                int &frameCounter, int forcedKeyframeInterval) {
    cv::Mat histOne, histTwo;

    calcHist(&frame, 1, nullptr, cv::Mat(), histOne, 1, &histSize, histRange);
    calcHist(&prevFrame, 1, nullptr, cv::Mat(), histTwo, 1, &histSize, histRange);

    normalize(histOne, histOne, 0, 1, cv::NORM_MINMAX);
    normalize(histTwo, histTwo, 0, 1, cv::NORM_MINMAX);

    cv::Mat diff;
    absdiff(histOne, histTwo, diff);

    double diffMean = mean(diff)[0];

    if (diffMean < threshold) {
        std::cout << "not a key frame!\n";
    } else {
        std::cout << "**key frame!**\n";
    }
    //cout << "Difference mean: " << diffMean << "\n";
    //cout << "Current threshold: " << threshold << "\n";

    if (threshold == 0) {
        cv::Scalar meanVal, stdDevVal;
        meanStdDev(diff, meanVal, stdDevVal);
        threshold = meanVal[0] + stdDevVal[0];
        std::cout << "New threshold set: " << threshold << "\n";
    }

    if (frameCounter && (frameCounter % forcedKeyframeInterval == 0)) {
        std::cout << "Forcing keyframe due to frame interval.\n";
        frameCounter = 0;
        return true;
    }

    if (diffMean > threshold) {
        frameCounter = 0;
    }

    return diffMean > threshold;
}


bool detectArtifacts(const cv::Mat &frame, const cv::Mat &prevFrame, std::vector<double> &buffer, const std::string &filename,
                     const double &threshold, int max_buffer_size, const float **histRange, const int &histSize) {
    cv::Mat histOne, histTwo;

    calcHist(&frame, 1, nullptr, cv::Mat(), histOne, 1, &histSize, histRange);
    calcHist(&prevFrame, 1, nullptr, cv::Mat(), histTwo, 1, &histSize, histRange);

    normalize(histOne, histOne, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());
    normalize(histTwo, histTwo, 0, 1, cv::NORM_MINMAX, -1, cv::Mat());

    double correlation = compareHist(histOne, histTwo, cv::HISTCMP_CORREL);

    if (buffer.size() < max_buffer_size) {
        buffer.push_back(correlation);
        return false;
    }
    double correlationAverage = cv::mean(buffer)[0];
    // cout << "Histogram correlation: " << correlationAverage << "\n";
    if (correlationAverage < threshold) {
        imwrite("prev" + filename, prevFrame);
        imwrite(filename, frame);
        std::cout << "Possible frame corruption detected due to low histogram correlation!\n";
    }
    buffer.clear();
    // cout << "Frames are similar, no corruption detected.\n";
    return correlationAverage < threshold;
}

bool detectColouredStripes(const cv::Mat &frame, const std::vector<ColorRange> &colorRanges, const double &threshold1,
                           const double &threshold2) {
    cv::Mat hsvFrame;
    cvtColor(frame, hsvFrame, cv::COLOR_BGR2HSV);
    cv::Mat combinedMask = cv::Mat::zeros(frame.size(), CV_8U);
    std::vector<double> colorDistributions = {};

    for (const auto &range: colorRanges) {
        cv::Mat mask;
        inRange(hsvFrame, range.lower, range.upper, mask);
        combinedMask |= mask;
        colorDistributions.push_back((mean(mask)[0] / 255) * 100);
    }

    cv::Scalar meanVal, stdDevVal;
    meanStdDev(colorDistributions, meanVal, stdDevVal);

    double scalingFactor = 100.0 / colorRanges.size();
    double colouredStripesProbability = (meanVal[0] / scalingFactor) * 100;

    std::cout << "combinedMask: " << colouredStripesProbability << "%\n";

    if (colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2) {
        imshow("coloured stripes", combinedMask);


    }
    return colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2;
}

bool detectBlackFrame(const cv::Mat &frame, const double &threshold) {
    std::cout << "mean: " << mean(frame)[0] << "\n";
    return mean(frame)[0] < threshold;
}

bool detectStaticFrame(const cv::Mat &frame, const cv::Mat &prevFrame, const double &threshold, std::vector<double> &buffer,
                       const int maxBufferSize) {
    cv::Mat diff;
    absdiff(frame, prevFrame, diff);
    double bufferAverage = 0;
    double avgDifference = mean(diff)[0];
    if (buffer.size() < maxBufferSize) {
        buffer.push_back(avgDifference);
        return false;
    }
    bufferAverage = cv::mean(buffer)[0];
    std::cout << bufferAverage << '\n';
    buffer.clear();
    return bufferAverage < threshold;
}//
// Created by gio on 10/23/24.
//
