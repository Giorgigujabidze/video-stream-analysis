//
// Created by gio on 12/1/24.
//


#include "capture.hpp"

#include <opencv2/core/mat.hpp>

#include "ffmpeg_capture.hpp"
#include "gstreamer_capture.hpp"

void Capture::resetCapture() {
    if (capture) {
        capture->releaseStream();
        capture.reset();
    }
}

int Capture::openStream(const std::string &url, const Config &config) {
    resetCapture();
    if (config.api_backend == 1900) {
        capture = std::make_unique<FFMpegCapture>();
    } else if (config.api_backend == 1800) {
        capture = std::make_unique<GstreamerCapture>();
    } else {
        return -1;
    }


    return capture->openStream(url, config, "30000000");
}

int Capture::grabFrame() const {
    return capture->grabFrame();
}

decode_status_t Capture::retrieveFrame(const bool keyframesOnly) const {
    return capture->retrieveFrame(keyframesOnly);
}

int Capture::getCVFrame(cv::Mat &frame) const {
    return capture->getCVFrame(frame);
}

void Capture::releaseStream() const {
    if (capture) {
        capture->releaseStream();
    }
}
