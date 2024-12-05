#include <iostream>
#include <opencv2/highgui.hpp>


#include <ffmpeg_capture.hpp>
#include <stream_analyzer.hpp>
#include "frame_analysis.hpp"


#include "helpers.hpp"

FFMpegCapture::FFMpegCapture(const std::string &url) {
    this->url = url;
}

FFMpegCapture::~FFMpegCapture() {
    release();
}


int FFMpegCapture::openStream(const std::string &url, const std::string &timeout) {
    av_dict_set(&options, "timeout", timeout.c_str(), 0);
    if (avformat_open_input(&pContext, url.c_str(), nullptr, &options) < 0) {
        std::cerr << "avformat_open_input error" << std::endl;
        return -1;
    }

    if (avformat_find_stream_info(pContext, nullptr) < 0) {
        std::cerr << "avformat_find_stream_info error" << std::endl;
        return -1;
    }

    videoStreamIndex = -1;
    for (int i = 0; i < pContext->nb_streams; i++) {
        AVCodecParameters *pLocalCodecParameters = nullptr;
        pLocalCodecParameters = pContext->streams[i]->codecpar;

        const AVCodec *pLocalCodec = nullptr;
        pLocalCodec = avcodec_find_decoder(pLocalCodecParameters->codec_id);
        if (pLocalCodec == nullptr) {
            std::cerr << "avcodec_find_decoder error" << std::endl;
            return -1;
        }

        if (pLocalCodecParameters->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStreamIndex = i;
            pCodec = pLocalCodec;
            pCodecParameters = pLocalCodecParameters;
        }

        std::cout << pLocalCodec->name << pLocalCodec->id << pLocalCodecParameters->bit_rate << std::endl;
    }

    if (videoStreamIndex == -1) {
        std::cerr << "couldn't find a video stream" << std::endl;
        return -1;
    }

    pCodecContext = avcodec_alloc_context3(pCodec);

    if (pCodecContext == nullptr) {
        std::cerr << "avcodec_alloc_context3 error" << std::endl;
        return -1;
    }

    if (avcodec_parameters_to_context(pCodecContext, pCodecParameters) < 0) {
        std::cerr << "avcodec_parameters_to_context() error" << std::endl;
        return -1;
    }

    if (avcodec_open2(pCodecContext, pCodec, nullptr) < 0) {
        std::cerr << "avcodec_open2 error" << std::endl;
        return -1;
    }

    pFrame = av_frame_alloc();

    if (pFrame == nullptr) {
        std::cerr << "av_frame_alloc error" << std::endl;
        delete pFrame;
        return -1;
    }

    pPacket = av_packet_alloc();

    if (pPacket == nullptr) {
        std::cerr << "av_packet_alloc error" << std::endl;
        delete pPacket;
        return -1;
    }
    return 0;
}

int FFMpegCapture::grabFrame() {
    if (av_read_frame(pContext, pPacket) < 0) {
        std::cerr << "av_read_frame error" << std::endl;
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


int FFMpegCapture::decodePacket(const AVPacket *pPacket, AVCodecContext *pCodecContext, AVFrame *pFrame) {
    int response = avcodec_send_packet(pCodecContext, pPacket);

    if (response < 0) {
        std::cerr << av_err2str(response) << std::endl;
        return DECODE_ERROR;
    }

    response = avcodec_receive_frame(pCodecContext, pFrame);

    if (response == AVERROR(EAGAIN) || response == AVERROR_EOF) {
        return NOT_ENOUGH_DATA;
    }
    if (response < 0) {
        std::cerr << "error receiving frame from the decoder" << std::endl;
        return DECODE_ERROR;
    }

    if (pFrame->decode_error_flags > 0) {
        return DECODE_ERROR;
    }

    return DECODE_OK;
}

int FFMpegCapture::getCVFrame(cv::Mat &frame) const {
    const int w = pFrame->width;
    const int h = pFrame->height;

    AVFrame *pFrameBGR = av_frame_alloc();
    if (pFrameBGR == nullptr) {
        std::cerr << "failed to allocate AVFrame for conversion." << std::endl;
        return -1;
    }

    pFrameBGR->format = AV_PIX_FMT_BGR24;
    pFrameBGR->width = w;
    pFrameBGR->height = h;

    if (av_frame_get_buffer(pFrameBGR, 0) < 0) {
        std::cerr << "failed to allocate buffer for AVFrame." << std::endl;
        av_frame_free(&pFrameBGR);
        return -1;
    }

    SwsContext *swsContext = sws_getContext(
        w, h, pCodecContext->pix_fmt,
        w, h, AV_PIX_FMT_BGR24,
        SWS_BILINEAR, nullptr, nullptr, nullptr);

    if (!swsContext) {
        std::cerr << "failed to initialize SwsContext." << std::endl;
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
}
