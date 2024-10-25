#include <opencv2/opencv.hpp>
#include <opencv2/quality/qualityssim.hpp>
#include "config.hpp"


double getSmoothedMSSIM(const std::vector<double> &mssimBuffer, int windowSize = 5) {
    double sum = 0;
    const int count = std::min(static_cast<int>(mssimBuffer.size()), windowSize);
    for (int i = mssimBuffer.size() - count; i < mssimBuffer.size(); i++) {
        sum += mssimBuffer[i];
    }
    return sum / count;
}


double calculateMSSIM(const cv::Mat &frame, const cv::Mat &prevFrame) {
    constexpr double C1 = 6.5025, C2 = 58.5225;
    constexpr int GAUSSIAN_SIZE = 11;
    constexpr double GAUSSIAN_SIGMA = 1.5;

    cv::Mat i1, i2;
    cv::Mat mu1, mu2;
    cv::Mat i1_squared, i2_squared, i1_i2;
    cv::Mat mu1_squared, mu2_squared, mu1_mu2;
    cv::Mat sigma1, sigma2, sigma12;
    cv::Mat t1, t2, t3, ssim_map;

    frame.convertTo(i1, CV_32F);
    prevFrame.convertTo(i2, CV_32F);

    cv::multiply(i1, i1, i1_squared);
    cv::multiply(i2, i2, i2_squared);
    cv::multiply(i1, i2, i1_i2);

    const cv::Size kernel_size(GAUSSIAN_SIZE, GAUSSIAN_SIZE);
    cv::GaussianBlur(i1, mu1, kernel_size, GAUSSIAN_SIGMA);
    cv::GaussianBlur(i2, mu2, kernel_size, GAUSSIAN_SIGMA);

    cv::multiply(mu1, mu1, mu1_squared);
    cv::multiply(mu2, mu2, mu2_squared);
    cv::multiply(mu1, mu2, mu1_mu2);

    cv::GaussianBlur(i1_squared, sigma1, kernel_size, GAUSSIAN_SIGMA);
    cv::subtract(sigma1, mu1_squared, sigma1);

    cv::GaussianBlur(i2_squared, sigma2, kernel_size, GAUSSIAN_SIGMA);
    cv::subtract(sigma2, mu2_squared, sigma2);

    cv::GaussianBlur(i1_i2, sigma12, kernel_size, GAUSSIAN_SIGMA);
    cv::subtract(sigma12, mu1_mu2, sigma12);

    cv::multiply(mu1_mu2, 2.0, t1);
    cv::add(t1, C1, t1);

    cv::multiply(sigma12, 2.0, t2);
    cv::add(t2, C2, t2);

    cv::multiply(t1, t2, t3);

    cv::add(mu1_squared, mu2_squared, t1);
    cv::add(t1, C1, t1);

    cv::add(sigma1, sigma2, t2);
    cv::add(t2, C2, t2);

    cv::multiply(t1, t2, t1);

    cv::divide(t3, t1, ssim_map);

    return cv::mean(ssim_map)[0];
}

double calculateMultiScaleMSSIM(const cv::Mat &frame, const cv::Mat &prevFrame) {
    static const std::vector weights = {0.0448, 0.2856, 0.3001, 0.2363, 0.1333};
    double mssim_score = 0.0;

    cv::Mat current_scale, prev_scale;
    frame.copyTo(current_scale);
    prevFrame.copyTo(prev_scale);

    for (const double weight: weights) {
        double mssim = calculateMSSIM(current_scale, prev_scale);
        mssim_score += mssim * weight;
        cv::resize(current_scale, current_scale,
                   cv::Size(current_scale.cols / 2, current_scale.rows / 2),
                   0, 0, cv::INTER_LINEAR);
        cv::resize(prev_scale, prev_scale,
                   cv::Size(prev_scale.cols / 2, prev_scale.rows / 2),
                   0, 0, cv::INTER_LINEAR);
    }

    return mssim_score;
}

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

    double scalingFactor = 100.0 / colorRanges.size();
    double colouredStripesProbability = (meanVal[0] / scalingFactor) * 100;

    std::cout << "combinedMask: " << colouredStripesProbability << "%\n";

    if (colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2) {
        cv::imshow("coloured stripes", combinedMask);
    }
    return colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2;
}

bool detectBlackFrame(const cv::Mat &frame, const double &threshold) {
    std::cout << "mean: " << mean(frame)[0] << "\n";
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
    std::cout << bufferAverage << '\n';
    buffer.clear();
    return bufferAverage < threshold;
} //
// Created by gio on 10/23/24.
//
