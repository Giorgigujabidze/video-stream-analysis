//
// Created by gio on 12/1/24.
//

#ifndef CAPTURE_HPP
#define CAPTURE_HPP
#include <opencv2/core/mat.hpp>

#include "ffmpeg_capture.hpp"
#include "icapture.hpp"


class Capture {
    std::unique_ptr<ICapture> capture;

public:
    void resetCapture();

    int openStream(const std::string &url, const Config &config);

    [[nodiscard]] int grabFrame() const;

    [[nodiscard]] decode_status_t retrieveFrame(bool keyframesOnly) const;

    [[nodiscard]] int getCVFrame(cv::Mat &frame) const;

    void releaseStream() const;
};


#endif //CAPTURE_HPP
