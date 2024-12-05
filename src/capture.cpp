//
// Created by gio on 12/1/24.
//


#include "capture.hpp"

#include <opencv2/core/mat.hpp>

#include "ffmpeg_capture.hpp"
#include "gstreamer_capture.hpp"

int Capture::openStream(const std::string &url, const int apiBackend) {
    if (apiBackend == 1900) {
        capture = new FFMpegCapture();
    } else if (apiBackend == 1800) {
        capture = new GstreamerCapture();
    } else {
        return -1;
    }

    return capture->openStream(url, "30000000");
}

int Capture::grabFrame() const {
    if (capture == nullptr) {
        return -1;
    }
    return capture->grabFrame();
}

int Capture::retrieveFrame(const bool keyframesOnly) const {
    if (capture == nullptr) {
        return -1;
    }
    return capture->retrieveFrame(keyframesOnly);
}

int Capture::getCVFrame(cv::Mat &frame) const {
    if (capture == nullptr) {
        return -1;
    }
    return capture->getCVFrame(frame);
}

void Capture::release() const {
    capture->release();
}
