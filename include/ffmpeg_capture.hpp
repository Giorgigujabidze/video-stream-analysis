//
// Created by gio on 11/26/24.
//

#ifndef FRAME_HPP
#define FRAME_HPP
#include <queue>
#include <string>

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
    std::string url;

    static int decodePacket(const AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame);

public:
    FFMpegCapture() = default;

    explicit FFMpegCapture(const std::string &url);

    ~FFMpegCapture() override;

    int openStream(const std::string &url, const std::string &timeout) override;

    [[nodiscard]] int grabFrame()  override;

    int retrieveFrame(bool keyframesOnly) override;

    int getCVFrame(cv::Mat &frame) const override;

    void release() override;
};


#endif //FRAME_HPP
