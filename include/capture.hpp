//
// Created by gio on 12/1/24.
//

#ifndef CAPTURE_HPP
#define CAPTURE_HPP
#include <opencv2/core/mat.hpp>

#include "icapture.hpp"


class Capture {
    ICapture *capture;

public:
    int openStream(const std::string &url, int apiBackend = 1900);

    [[nodiscard]] int grabFrame() const;

    [[nodiscard]] int retrieveFrame(bool keyframesOnly) const;

    [[nodiscard]] int getCVFrame(cv::Mat &frame) const;

    void release() const;
};

#endif //CAPTURE_HPP
