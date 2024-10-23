#include <opencv2/opencv.hpp>
#include <opencv2/quality/qualityssim.hpp>
#include "config.hpp"


bool isKeyFrame(const cv::Mat &frame, const cv::Mat &prevFrame, double &threshold, const float **histRange,
                const int &histSize,
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

double getMSSIM(const cv::Mat &frame, const cv::Mat &prevFrame) {
    const double C1 = 6.5025, C2 = 58.5225;
    cv::Mat i1, i2;

    frame.convertTo(i1, CV_32F);
    prevFrame.convertTo(i2, CV_32F);

    cv::Mat i1Squared = i1.mul(i1);
    cv::Mat i2Squared = i2.mul(i2);
    cv::Mat i1Xi2 = i1.mul(i2);

    cv::Mat mu1, mu2;
    cv::GaussianBlur(i1, mu1, cv::Size(11, 11), 1.5);
    cv::GaussianBlur(i2, mu2, cv::Size(11, 11), 1.5);

    cv::Mat mu1Squared = mu1.mul(mu1);
    cv::Mat mu2Squared = mu2.mul(mu2);
    cv::Mat mu1Xmu2 = mu1.mul(mu2);

    cv::Mat sigma1, sigma2, sigma12;

    cv::GaussianBlur(i1Squared, sigma1, cv::Size(11, 11), 1.5);
    sigma1 -= mu1Squared;
    cv::GaussianBlur(i2Squared, sigma2, cv::Size(11, 11), 1.5);
    sigma2 -= mu2Squared;
    cv::GaussianBlur(i1Xi2, sigma12, cv::Size(11, 11), 1.5);
    sigma12 -= mu1Xmu2;

    cv::Mat t1, t2, t3;

    t1 = 2 * mu1Xmu2 + C1;
    t2 = 2 * sigma12 + C2;
    t3 = t1.mul(t2);

    t1 = mu1Squared + mu2Squared + C1;
    t2 = sigma1 + sigma2 + C2;
    t1 = t1.mul(t2);

    cv::Mat ssimMap;

    cv::divide(t3, t1, ssimMap);

    cv::Scalar mssim = cv::mean(ssimMap);

    return mssim[0];

}


bool detectArtifacts(const cv::Mat &frame, const cv::Mat &prevFrame, std::vector<double> &buffer,
                     const std::string &filename,
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

bool
detectStaticFrame(const cv::Mat &frame, const cv::Mat &prevFrame, const double &threshold, std::vector<double> &buffer,
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
