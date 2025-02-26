#include "stream_analyzer.hpp"

#include <atomic>
#include <fstream>

#include "capture.hpp"
#include "ffmpeg_capture.hpp"
#include "frame_analysis.hpp"
#include "helpers.hpp"
#include "metrics.hpp"
#include "threading.hpp"

void *analyzeVideoStream(void *threadArgs) {
    auto args = static_cast<ThreadArguments *>(threadArgs);
    Capture cap;

    auto cleanup = [args, &cap] {
        delete args;
        cap.resetCapture();
    };

    if (openVideoStream(cap, args->stream.url, args->config) < 0) {
        cleanup();
        return nullptr;
    }

    std::string imgName = "../images/img" + args->stream.name + ".png";
    std::string filename = "../results/results" + args->stream.name + ".json";

    cv::Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    std::vector<double> meanBuffer;

    auto start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    auto metrics = Metrics{};

    auto grab = [&cap, &args, &metrics]() {
        if (cap.grabFrame() < 0) {
            log(args->config, "blank frame grabbed " + args->stream.url, ERROR);
            metrics.blank_frame_count++;
            return false;
        }
        return true;
    };

    auto resetData = ResetData{
        .frameCount = frameCount,
        .start = start,
        .meanBuffer = meanBuffer,
        .metrics = metrics
    };

    auto saveData = SaveData{
        .filename = filename,
        .imageName = imgName,
        .metrics = metrics,
        .arguments = args,
        .image = frame
    };

    auto analysisData = AnalysisData{
        .frame = grayFrame,
        .prevFrame = prevGrayFrame,
        .downscaledFrame = downscaledFrame,
        .meanBuffer = meanBuffer,
        .args = args,
        .metrics = metrics
    };

    ReconnectData reconnectData = ReconnectData{
        .filename = filename,
        .metrics = metrics,
        .cap = cap,
        .config = args->config,
        .url = args->stream.url,
        .stopFlag = args->stopFlag
    };

    while (!args->stopFlag->load()) {
        if (metrics.blank_frame_count > 10) {
            if (reconnect(reconnectData) < 0) {
                cleanup();
                return nullptr;
            }
            continue;
        }

        if (!grab()) {
            continue;
        }

        if (frameCount % args->config.process_every_nth_frame == 0) {
            decode_status_t resp = cap.retrieveFrame(args->config.key_frames_only);
            if (!handleDecodeResponse(resp, metrics, args)) {
                continue;
            }

            if (cap.getCVFrame(frame) < 0) {
                log(args->config, "getCVFrame error " + args->stream.url, ERROR);
                continue;
            }

            if (frame.empty()) {
                log(args->config, "empty frame " + args->stream.url, ERROR);
                metrics.blank_frame_count++;
                continue;
            }

            preprocessFrame(frame, downscaledFrame, grayFrame);

            if (prevGrayFrame.empty()) {
                prevGrayFrame = grayFrame.clone();
                continue;
            }

            analyzeFrames(analysisData);
            prevGrayFrame = grayFrame.clone();
        }

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);

        if (duration.count() >= args->config.interval) {
            if (!onIntervalElapsed(resetData, saveData)) {
                cleanup();
                return nullptr;
            }
        }

        frameCount++;
    }
    cleanup();
    return nullptr;
}


void analyzeFrames(AnalysisData &data) {
    auto sData = StaticFrameData{
        .frame = data.frame,
        .prevFrame = data.prevFrame,
        .threshold = data.args->config.thresholds.static_frame_threshold,
        .buffer = data.meanBuffer,
        .maxBufferSize = data.args->config.size_parameters.max_mean_buffer_size
    };

    auto bData = BlackFrameData{
        .frame = data.frame,
        .threshold = data.args->config.thresholds.black_frame_threshold
    };

    auto cData = ColouredStripesData{
        .frame = data.downscaledFrame,
        .colorRanges = data.args->colorRanges,
        .threshold1 = data.args->config.thresholds.coloured_stripes_threshold,
        .threshold2 = data.args->config.thresholds.coloured_stripes_max_deviation
    };

    if (detectStaticFrame(sData)) {
        data.metrics.static_frame_count++;

        if (detectBlackFrame(bData)) {
            data.metrics.black_frame_count++;
        } else if (!data.metrics.coloured_stripes_detected && detectColouredStripes(cData)) {
            data.metrics.coloured_stripes_detected = true;
        }
    }
}
