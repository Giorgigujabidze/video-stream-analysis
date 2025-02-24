#include <iostream>
#include <opencv2/highgui.hpp>
#include <libavutil/time.h>
#include <ffmpeg_capture.hpp>
#include <stream_analyzer.hpp>
#include "frame_analysis.hpp"


#include "helpers.hpp"

FFMpegCapture::~FFMpegCapture() {
    release();
}

int FFMpegCapture::openStream(const std::string &url, const Config &config, const std::string &timeout) {
    this->config = config;
    const std::string modifiedUrl = modifyUrlForMulticast(url);

    av_dict_set(&options, "timeout", timeout.c_str(), 0);

    pContext = avformat_alloc_context();

    if (pContext == nullptr) {
        log(config, "failed to allocate AVFormatContext", ERROR);
        return -1;
    }


    if (avformat_open_input(&pContext, modifiedUrl.c_str(), nullptr, &options) < 0) {
        log(config, "failed to open stream", ERROR);
        return -1;
    }

    setInterruptCallback();

    if (avformat_find_stream_info(pContext, nullptr) < 0) {
        log(config, "failed to find stream information", ERROR);
        return -1;
    }

    unsetInterruptCallback();

    if (std::chrono::steady_clock::now() - timer > timeoutDuration) {
        log(config, "stream timeout", ERROR);
        return -1;
    }

    videoStreamIndex = -1;
    for (int i = 0; i < pContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = nullptr;
        pLocalCodecParameters = pContext->streams[i]->codecpar;
        const AVCodec *pLocalCodec = nullptr;

        if (pLocalCodecParameters->codec_type != AVMEDIA_TYPE_VIDEO) {
            continue;
        }
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);

        if (pLocalCodec == nullptr) {
            log(config, "failed to find codec", ERROR);
            return -1;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            pCodec = pLocalCodec;
            pCodecParameters = pLocalCodecParameters;
        }
        log(config, std::string(pLocalCodec->name) + " " +
                    std::to_string(pLocalCodec->id) + " " +
                    std::to_string(pLocalCodecParameters->bit_rate), INFO);
    }

    if (videoStreamIndex == -1) {
        log(config, "couldn't find a video stream", ERROR);
        return -1;
    }

    pCodecContext = avcodec_alloc_context3(pCodec);

    if (pCodecContext == nullptr) {
        log(config, "failed to allocate AVCodecContext", ERROR);
        return -1;
    }

    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        log(config, "failed to copy codec parameters", ERROR);
        return -1;
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        log(config, "failed to open codec", ERROR);
        return -1;
    }

    pFrame = av_frame_alloc();

    if (pFrame == nullptr) {
        log(config, "failed to allocate video frame", ERROR);
        delete pFrame;
        return -1;
    }

    pPacket = av_packet_alloc();

    if (pPacket == nullptr) {
        log(config, "failed to allocate AVPacket", ERROR);
        delete pPacket;
        return -1;
    }
    return 0;
}

int FFMpegCapture::grabFrame() {
    if (av_read_frame(pContext, pPacket) < 0) {
        log(config, "failed to read frame", ERROR);
        av_packet_unref(pPacket);
        return -1;
    }

    return 0;
}

int FFMpegCapture::retrieveFrame(const bool keyframesOnly) {
    if (pPacket->stream_index == videoStreamIndex &&
        (!keyframesOnly || (pPacket->flags & AV_PKT_FLAG_KEY))) {
        response = decodePacket(pPacket, pCodecContext, pFrame);
        av_packet_unref(pPacket);
        return response;
    }
    av_packet_unref(pPacket);
    return NON_VIDEO_PACKET;
}


void FFMpegCapture::setInterruptCallback() {
    timer = std::chrono::steady_clock::now();
    pContext->interrupt_callback.callback = interruptCallback;
    pContext->interrupt_callback.opaque = this;
}

void FFMpegCapture::unsetInterruptCallback() const {
    pContext->interrupt_callback.callback = nullptr;
    pContext->interrupt_callback.opaque = nullptr;
}

std::string FFMpegCapture::modifyUrlForMulticast(const std::string &url) {
    if (url.substr(0, 6) == "rtp://") {
        return "udp://" + url.substr(6);
    }
    return url;
}

int FFMpegCapture::decodePacket(const AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) const {
    int response = avcodec_send_packet(pCodecContext, pPacket);

    if (response < 0) {
        log(config, "failed to send packet", ERROR);
        return DECODE_ERROR;
    }

    response = avcodec_receive_frame(pCodecContext, pFrame);

    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        return NOT_ENOUGH_DATA;
    }
    if (response < 0) {
        log(config, "failed to decode frame", ERROR);
        return DECODE_ERROR;
    }

    if (pFrame->decode_error_flags > 0) {
        return DECODE_ERROR;
    }

    return DECODE_OK;
}

// TODO: future optimization needed

int FFMpegCapture::getCVFrame(cv::Mat &frame) const {
    const int w = pFrame->width;
    const int h = pFrame->height;

    AVFrame *pFrameBGR = av_frame_alloc();
    if (pFrameBGR == nullptr) {
        log(config, "failed to allocate AVFrame", ERROR);
        return -1;
    }

    pFrameBGR->format = AV_PIX_FMT_BGR24;
    pFrameBGR->width = w;
    pFrameBGR->height = h;

    if (av_frame_get_buffer(pFrameBGR, 0) < 0) {
        log(config, "failed to allocate buffer for AVFrame.", ERROR);
        av_frame_free(&pFrameBGR);
        return -1;
    }

    SwsContext *swsContext = sws_getContext(
        w, h, pCodecContext->pix_fmt,
        w, h, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!swsContext) {
        log(config, "failed to initialize SwsContext.", ERROR);
        av_frame_free(&pFrameBGR);
        return -1;
    }

    sws_scale(swsContext, pFrame->data, pFrame->linesize, 0, h,
              pFrameBGR->data, pFrameBGR->linesize);

    const cv::Mat mat(h, w, CV_8UC3, pFrameBGR->data[0], pFrameBGR->linesize[0]);
    mat.copyTo(frame);

    av_frame_free(&pFrameBGR);
    sws_freeContext(swsContext);

    return 0;
}

void FFMpegCapture::release() {
    if (pPacket) {
        av_packet_free(&pPacket);
    }
    if (pFrame) {
        av_frame_free(&pFrame);
    }
    if (pCodecContext) {
        avcodec_free_context(&pCodecContext);
    }
    if (options) {
        av_dict_free(&options);
    }

    if (pContext) {
        avformat_close_input(&pContext);
    }
    options = nullptr;
    pCodec = nullptr;
    pCodecParameters = nullptr;
    videoStreamIndex = -1;
    response = 0;
    config = {};
}
