#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <thread>
#include <chrono>


int main(const int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "You should specify config file and result file paths";
        return -1;
    }

    auto config = Config{};
    if (loadConfigFromJson(argv[1], config) < 0) {
        std::cout << "failed to load config file\n";
        return -1;
    }

    const std::vector<ColorRange> colorRanges = loadColorRangesFromJson(config.colorRangesPath);
    if (colorRanges.empty()) {
        std::cout << "failed to load color ranges\n";
        return -1;
    }
    std::vector<double> meanBuffer = {};
    std::vector<double> correlationBuffer = {};
    cv::VideoCapture cap;

    if (!cap.open(config.url, cv::CAP_FFMPEG, {cv::CAP_PROP_HW_ACCELERATION, cv::VIDEO_ACCELERATION_VAAPI})) {
        std::cerr << "Failed to open video stream\n";
        return -1;
    }

    std::cout << "Starting to grab frames\n";
    while (true) {
        auto metrics = Metrics{};
        analyzeVideoStream(cap, config, colorRanges, metrics, meanBuffer);
        printMetrics(metrics);
        writeResultsToCSV(argv[2], metrics);
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }

    return 0;
}

