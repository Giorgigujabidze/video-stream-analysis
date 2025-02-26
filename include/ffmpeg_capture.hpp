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


class FFMpegCapture final : public ICapture {
    AVFormatContext *pContext = nullptr;
    AVDictionary *options = nullptr;
    const AVCodec *pCodec = nullptr;
    AVCodecParameters *pCodecParameters = nullptr;
    AVCodecContext *pCodecContext = nullptr;
    AVFrame *pFrame = nullptr;
    AVPacket *pPacket = nullptr;
    int videoStreamIndex = -1;
    decode_status_t response = DECODE_OK;
    Config config{};

    std::chrono::steady_clock::time_point timer;
    std::chrono::seconds timeoutDuration = std::chrono::seconds(30);

    void setStreamOptions(const std::string &timeout);

    static int interruptCallback(void *ctx);

    void resetTimer();

    void setInterruptCallback();

    void unsetInterruptCallback() const;

    static std::string modifyUrlForMulticast(const std::string &url);

    decode_status_t decodePacket(const AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) const;

public:
    FFMpegCapture() = default;

    ~FFMpegCapture() override;

    int openStream(const std::string &url, const Config &config, const std::string &timeout) override;

    [[nodiscard]] int grabFrame() override;

    decode_status_t retrieveFrame(bool keyframesOnly) override;

    int getCVFrame(cv::Mat &frame) const override;

    void releaseStream() override;
};


#endif //FRAME_HPP
