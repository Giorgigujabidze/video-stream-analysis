#include "stream_analyzer.hpp"
#include <fstream>
#include "frame_analysis.hpp"
#include "metrics.hpp"
#include "threading.hpp"
#include "helpers.hpp"


void *analyzeVideoStream(void *threadArgs) {
    const auto args = static_cast<ThreadArguments *>(threadArgs);
    cv::VideoCapture cap;

    if (openVideoStream(cap, args->config) < 0) {
        return nullptr;
    }

    std::string filename = "../results/results" + args->config.name + ".json";
    cv::Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    auto start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    std::vector<double> meanBuffer;
    auto metrics = Metrics{};

    while (true) {
        if (metrics.blank_frame_count > 10) {
            reconnect(filename, metrics, cap, args->config);
            continue;
        }

        if (!cap.grab()) {
            std::cerr << "blank frame grabbed\n";
            metrics.blank_frame_count++;
            continue;
        }

        if (frameCount % args->config.process_every_nth_frame == 0) {
            cap.retrieve(frame);

            if (frame.empty()) {
                std::cerr << "empty frame\n";
                metrics.blank_frame_count++;
                continue;
            }

            preprocessFrame(frame, downscaledFrame, grayFrame);

            if (prevGrayFrame.empty()) {
                prevGrayFrame = grayFrame.clone();
                continue;
            }
            analyzeFrames(grayFrame, prevGrayFrame, downscaledFrame, meanBuffer, metrics, args);
            prevGrayFrame = grayFrame.clone();
        }
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);

        if (duration.count() >= args->config.interval) {
            saveAndReset(filename, metrics, frameCount, meanBuffer, start);
        }

        frameCount++;
    }
}


void analyzeFrames(const cv::Mat &grayFrame, const cv::Mat &prevGrayFrame, const cv::Mat &downscaledFrame,
                   std::vector<double> &meanBuffer, Metrics &metrics, const ThreadArguments *args) {
    if (detectStaticFrame(grayFrame, prevGrayFrame, args->config.thresholds.static_frame_threshold,
                          meanBuffer,
                          args->config.size_parameters.max_mean_buffer_size)) {
        metrics.static_frame_count++;
        if (detectBlackFrame(grayFrame, args->config.thresholds.black_frame_threshold)) {
            metrics.black_frame_count++;
        } else if (!metrics.coloured_stripes_detected &&
                   detectColouredStripes(downscaledFrame, args->colorRanges,
                                         args->config.thresholds.coloured_stripes_threshold,
                                         args->config.thresholds.coloured_stripes_max_deviation)) {
            metrics.coloured_stripes_detected = true;
        }
    }
}
