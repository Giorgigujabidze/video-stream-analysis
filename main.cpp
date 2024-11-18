#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <thread>
#include <chrono>




int main(const int argc, char **argv) {
    if (argc < 3) {
        std::cerr << "Incorrect number of arguments" << std::endl
        << "Usages: " << std::endl
        << argv[0] << " <config_file> <output_file>" << std::endl
        << argv[0]<< " <-c> <data_file>" << std::endl;
        return -1;
    }

    if (std::string(argv[1]) == "-c") {
        std::vector<StreamData> streamDataVector;
        readDataFromFile(argv[2], streamDataVector);
        configMaker("../config/config.json", streamDataVector);
        std::cout << "configs where successfully generated\n";
        return 0;
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
        if (config.output_to_console) {
            printMetrics(metrics);
        }
        writeResultsToCSV(argv[2], metrics, config.max_log_number);
        if (resp < 0) {
            cap.release();
            openVideoStream(cap, config);
        }
        std::this_thread::sleep_for(std::chrono::seconds(3));
    }
    return 0;
}

