//
// Created by gio on 11/26/24.
//

#ifndef FRAME_HPP
#define FRAME_HPP
#include <iostream>
#include <string>
#include <chrono>
#include "icapture.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

enum RETRIEVE_FLAGS {
    DECODE_OK = 70,
    DECODE_ERROR = 80,
    NOT_ENOUGH_DATA = 90,
    NON_VIDEO_PACKET = 100
};

class FFMpegCapture final : public ICapture {
    AVFormatContext *pContext = nullptr;
    AVDictionary *options = nullptr;
    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;
    AVCodecContext *pCodecContext = nullptr;
    AVFrame *pFrame = nullptr;
    AVPacket *pPacket = nullptr;
    int videoStreamIndex = -1;
    int response = 0;
    Config config{};

    std::chrono::steady_clock::time_point timer;
    std::chrono::seconds timeoutDuration = std::chrono::seconds(30);


    static int interruptCallback(void *ctx) {
        const auto *capture = static_cast<FFMpegCapture *>(ctx);
        const auto now = std::chrono::steady_clock::now();
        const auto duration = std::chrono::duration_cast<std::chrono::seconds>(
            now - capture->timer
        );

        return duration > capture->timeoutDuration ? 1 : 0;
    }


    void resetTimer() {
        timer = std::chrono::steady_clock::now();
    }


    void setInterruptCallback();

    void unsetInterruptCallback() const;

    static std::string modifyUrlForMulticast(const std::string &url);

    int decodePacket(const AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) const;

public:
    FFMpegCapture() = default;

    ~FFMpegCapture() override;

    int openStream(const std::string &url, const Config &config, const std::string &timeout) override;

    [[nodiscard]] int grabFrame() override;

    int retrieveFrame(bool keyframesOnly) override;

    int getCVFrame(cv::Mat &frame) const override;

    void release() override;
};


#endif //FRAME_HPP
