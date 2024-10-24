#include <opencv2/opencv.hpp>
#include <opencv2/quality/qualityssim.hpp>
#include "config.hpp"


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
