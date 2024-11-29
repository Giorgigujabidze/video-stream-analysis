#include "stream_analyzer.hpp"
#include <fstream>

#include "frame.hpp"
#include "frame_analysis.hpp"
#include "metrics.hpp"
#include "threading.hpp"
#include "helpers.hpp"


void *analyzeVideoStream(void *threadArgs) {
    const auto args = static_cast<ThreadArguments *>(threadArgs);
    Capture cap;

    if (openVideoStream(cap, args->stream.url) < 0) {
        return nullptr;
    }

    std::string imgName = "../images/img" + args->stream.name + ".png";
    std::string filename = "../results/results" + args->stream.name + ".json";

    cv::Mat downscaledFrame, prevGrayFrame, grayFrame;
    auto start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    std::vector<double> meanBuffer;
    auto metrics = Metrics{};

    while (true) {
        if (metrics.blank_frame_count > 10) {
            if (reconnect(filename, metrics, cap, args->config, args->stream.url) < 0) {
                return nullptr;
            }
            continue;
        }

        if (cap.grabFrame() < 0) {
            std::cerr << "blank frame grabbed\n";
            metrics.blank_frame_count++;
            continue;
        }

        if (frameCount % args->config.process_every_nth_frame == 0) {
            cv::Mat frame;
            if (cap.retrieveFrame() != DECODE_OK) {
                continue;
            }

            cap.getCVFrame(frame);

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

        cv::imshow("window" + args->stream.name, downscaledFrame);

        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);


        if (cv::waitKey(1) >= 0 || duration.count() >= args->config.interval) {
            if (saveAndReset(filename, metrics, frameCount, meanBuffer, start) < 0) {
                return nullptr;
            }
            if (args->config.save_last_frame) {
                if (!imwrite(imgName, downscaledFrame)) {
                    std::cout << imgName << " save failed\n";
                    return nullptr;
                }
            }
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
