//
// Created by gio on 10/23/24.
//

#ifndef STREAM_ANALYSIS_CONFIG_HPP
#define STREAM_ANALYSIS_CONFIG_HPP


#include <string>
#include <opencv2/core/types.hpp>
#include <nlohmann/json.hpp>

struct Thresholds {
    double static_frame_threshold;
    double coloured_stripes_threshold;
    double coloured_stripes_max_deviation;
    double black_frame_threshold;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Thresholds, static_frame_threshold, coloured_stripes_threshold,
                                   coloured_stripes_max_deviation, black_frame_threshold);
};

struct SizeParameters {
    int max_mean_buffer_size;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(SizeParameters, max_mean_buffer_size);
};

struct Config {
    std::string url;
    std::string color_ranges_path;
    int api_backend;
    int hardware_acceleration;
    int process_every_nth_frame;
    int max_log_number;
    Thresholds thresholds;
    SizeParameters size_parameters;
    time_t interval;
    bool output_to_console;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, url, color_ranges_path,api_backend,
                                   hardware_acceleration,process_every_nth_frame,max_log_number, thresholds, size_parameters, interval, output_to_console);
};

struct ColorRange {
    std::string name;
    cv::Scalar lower;
    cv::Scalar upper;
};

int loadConfigFromJson(const std::string &filename, Config &config);

std::vector<ColorRange> loadColorRangesFromJson(const std::string &filename);

#endif //STREAM_ANALYSIS_CONFIG_HPP
