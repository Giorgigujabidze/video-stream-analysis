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

    const std::vector<ColorRange> colorRanges = loadColorRangesFromJson(config.color_ranges_path);
    if (colorRanges.empty()) {
        std::cout << "failed to load color ranges\n";
        return -1;
    }

    std::vector<double> meanBuffer = {};
    std::vector<double> correlationBuffer = {};
    cv::VideoCapture cap;

    openVideoStream(cap,config);

    std::cout << "Starting to grab frames\n";
    while (true) {
        auto metrics = Metrics{};
        const int resp = analyzeVideoStream(cap, config, colorRanges, metrics, meanBuffer);
        printMetrics(metrics);
        writeResultsToCSV(argv[2], metrics, config.max_log_number);
        if (resp < 0) {
            cap.release();
            openVideoStream(cap, config);
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    return 0;
}

