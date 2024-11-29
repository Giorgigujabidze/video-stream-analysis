//
// Created by gio on 11/26/24.
//

#ifndef FRAME_HPP
#define FRAME_HPP
#include <queue>
#include <string>

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

class Capture {
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
    Capture() = default;

    explicit Capture(const std::string &url);

    ~Capture();

    int grabFrame() const;

    int retrieveFrame();

    int openStream(const std::string &url, const std::string &timeout = "30000000");

    int getCVFrame(cv::Mat &frame) const;

    void release();
};


#endif //FRAME_HPP
