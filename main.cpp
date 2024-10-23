#include "stream_analyzer.hpp"
#include "frame_analysis.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <thread>
#include <chrono>

using namespace std;
using namespace cv;

int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "You should specify config file and result file paths";
        return -1;
    }

    Config config = Config{};
    if (loadConfigFromJson(argv[1], config) < 0) {
        cout << "failed to load config file\n";
        return -1;
    }

    vector<ColorRange> colorRanges = loadColorRangesFromJson(config.colorRangesPath);
    if (colorRanges.empty()) {
        cout << "failed to load color ranges\n";
        return -1;
    }
    vector<double> meanBuffer = {};
    vector<double> correlationBuffer = {};
    VideoCapture cap;

    float range[] = {0, static_cast<float>(config.sizeParameters.histogramSize)};
    const float *histRange = {range};

    if (!cap.open(config.url, CAP_FFMPEG, {CAP_PROP_HW_ACCELERATION, VIDEO_ACCELERATION_VAAPI})) {
        cerr << "Failed to open video stream\n";
        return -1;
    }
    //cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);

    cout << "Starting to grab frames\n";
    while (true) {
        auto metrics = Metrics{};
        analyzeVideoStream(cap, config, colorRanges, metrics, meanBuffer, correlationBuffer, &histRange);
        printMetrics(metrics);
        writeResultsToJson(argv[2], metrics);
        this_thread::sleep_for(chrono::seconds(3));
    }

    return 0;
}

