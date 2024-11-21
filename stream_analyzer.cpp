#include "stream_analyzer.hpp"
#include "frame_analysis.hpp"
#include "metrics.hpp"
#include "threading.hpp"


int openVideoStream(cv::VideoCapture &cap, Config &config) {
    if (!cap.open(config.url, config.api_backend, {cv::CAP_PROP_HW_ACCELERATION, config.hardware_acceleration})) {
        std::cerr << "failed to open video stream\n";
        cap.release();
        return -1;
    }
    return 0;
}


void *analyzeVideoStream(void *threadArgs) {
    const auto args = static_cast<ThreadArguments *>(threadArgs);
    cv::VideoCapture cap;
    if (openVideoStream(cap, args->config) < 0) {
        return nullptr;
    }
    std::string filename = "../results/results" + args->config.name + ".json";
    std::cout << args->config.url << "\n";
    cv::Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    auto start = std::chrono::high_resolution_clock::now();
    int frameCount = 0;
    std::vector<double> meanBuffer;
    auto metrics = Metrics{};

    while (true) {
        if (metrics.blank_frame_count > 10) {
            metrics.no_input_stream = true;
            std::cerr << "connection lost. attempting to reconnect..." << std::endl;
            cap.release();
            writeResultsToJson(filename, metrics);
            while (true) {
                if (openVideoStream(cap, args->config) >= 0) {
                    std::cout << "reconnected\n";
                    metrics = Metrics{};
                    break;
                }
                std::cerr << "failed to reconnect\n";
                sleep(5);
            }
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

            resize(frame, downscaledFrame, cv::Size(600, 400), 0, 0, cv::INTER_LINEAR);
            cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);

            if (prevGrayFrame.empty()) {
                prevGrayFrame = grayFrame.clone();
                continue;
            }

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
            prevGrayFrame = grayFrame.clone();
            cv::imshow("win" + args->config.name, downscaledFrame);
        }
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);
        if (cv::waitKey(1) >= 1 || duration.count() >= args->config.interval) {
            writeResultsToJson(filename, metrics);
            frameCount = 0;
            meanBuffer.erase(meanBuffer.begin(), meanBuffer.end());
            metrics = Metrics{};
            start = std::chrono::high_resolution_clock::now();
        }
        frameCount++;
    }
}
