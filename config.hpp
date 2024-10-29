//
// Created by gio on 10/23/24.
//

#ifndef STREAM_ANALYSIS_CONFIG_HPP
#define STREAM_ANALYSIS_CONFIG_HPP


#include <string>
#include <opencv2/core/types.hpp>
#include <nlohmann/json.hpp>

struct Thresholds {
    double staticFrameThreshold;
    double colouredStripesThreshold;
    double colouredStripesMaxDeviation;
    double blackFrameThreshold;
};

struct SizeParameters {
    int maxMeanBufferSize;
};

struct Config {
    std::string url;
    std::string colorRangesPath;
    std::string corruptedFramesPath;
    int hardware_acceleration;
    int max_log_number;
    Thresholds thresholds;
    SizeParameters sizeParameters;
    time_t interval;
};

struct ColorRange {
    std::string name;
    cv::Scalar lower;
    cv::Scalar upper;
};

int loadConfigFromJson(const std::string& filename, Config& config);
std::vector<ColorRange> loadColorRangesFromJson(const std::string& filename);

#endif //STREAM_ANALYSIS_CONFIG_HPP
