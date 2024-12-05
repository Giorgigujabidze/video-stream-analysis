//
// Created by gio on 12/2/24.
//

#ifndef GSTREAMER_CAPTURE_HPP
#define GSTREAMER_CAPTURE_HPP

#include <iostream>

#include "capture.hpp"

extern "C" {
#include <gst/gst.h>
#include <gst/app/gstappsink.h>
}

inline void checkErr(const GError *err) {
    if (err) {
        std::cerr << "checkErr : " << err->message << std::endl;
        exit(0);
    }
}


class GstreamerCapture final : public ICapture {
public:
    ~GstreamerCapture() override;

    int openStream(const std::string &url, const std::string &timeout) override;

    [[nodiscard]] int grabFrame() override;

    int retrieveFrame(bool keyframesOnly) override;

    int getCVFrame(cv::Mat &frame) const override;

    void release() override;

private:
    int width = 0, height = 0;
    GstElement *pipeline = nullptr;
    GstElement *sinkVideo = nullptr;
    GstSample *sample = nullptr;
    GstBus *bus = nullptr;
    GstCaps *caps = nullptr;
    GstStructure *structure = nullptr;
    GstBuffer *buffer = nullptr;


    static int processBusMessage(GstElement *pipeline, GstMessage *msg);
};


#endif //GSTREAMER_CAPTURE_HPP
