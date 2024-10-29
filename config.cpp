#include "config.hpp"
#include <fstream>
#include <iostream>


int loadConfigFromJson(const std::string &filename, Config &config) {
    std::ifstream file(filename);
    nlohmann::json j;

    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cerr << e.what();
        return -1;
    }

    config.url = j["url"];
    config.colorRangesPath = j["color_ranges_path"];
    config.corruptedFramesPath = j["corrupted_frames_path"];
    config.hardware_acceleration = j["hardware_acceleration"];
    config.max_log_number = j["max_log_number"];
    config.interval = j["interval"];
    config.thresholds = {
            j["thresholds"]["static_frame_threshold"],
            j["thresholds"]["coloured_stripes_threshold"],
            j["thresholds"]["coloured_stripes_max_deviation"],
            j["thresholds"]["black_frame_threshold"],
    };
    config.sizeParameters = {
            j["size_parameters"]["max_mean_buffer_size"],
    };
    return 0;
}

std::vector<ColorRange> loadColorRangesFromJson(const std::string &filename) {
    std::ifstream file(filename);
    nlohmann::json  j;
    std::vector<ColorRange> colorRanges;

    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cout << e.what() << '\n';
        return colorRanges;
    }

    for (auto &range: j["colorRanges"]) {
        colorRanges.push_back({
                                      range["name"],
                                      cv::Scalar(range["lower"]["h"], range["lower"]["s"], range["lower"]["v"]),
                                      cv::Scalar(range["upper"]["h"], range["upper"]["s"], range["upper"]["v"])
                              });
    }
    return colorRanges;
}