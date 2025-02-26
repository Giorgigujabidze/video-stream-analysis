//
// Created by gio on 12/1/24.
//

#ifndef ICAPTURE_HPP
#define ICAPTURE_HPP
#include <string>

#include "config.hpp"
#include "ffmpeg_capture.hpp"


typedef enum DecodeStatus {
    DECODE_OK = 70,
    DECODE_ERROR = 80,
    NOT_ENOUGH_DATA = 90,
    NON_VIDEO_PACKET = 100
} decode_status_t;


typedef enum ReadFrameStatus {

}read_status_t;

class ICapture {
public:
    virtual ~ICapture() = default;

    virtual int openStream(const std::string &url, const Config &config, const std::string &timeout) = 0;

    [[nodiscard]] virtual int grabFrame() = 0;

    virtual decode_status_t retrieveFrame(bool keyframesOnly) = 0;

    virtual int getCVFrame(cv::Mat &frame) const = 0;

    virtual void releaseStream() = 0;
};


#endif //ICAPTURE_HPP
