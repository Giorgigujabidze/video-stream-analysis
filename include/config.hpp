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
    int api_backend;
    int hardware_acceleration;
    int process_every_nth_frame;
    Thresholds thresholds;
    SizeParameters size_parameters;
    time_t interval;
    bool output_to_console;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, api_backend,
                                   hardware_acceleration, process_every_nth_frame, thresholds,
                                   size_parameters, interval, output_to_console);
};

struct Stream {
    std::string name;
    std::string url;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Stream, name, url);
};


struct Streams {
    std::vector<Stream> streams;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Streams, streams);
};

struct ColorRange {
    std::string name;
    cv::Scalar lower;
    cv::Scalar upper;
};

struct StreamData {
    std::string lid;
    std::string name;
    std::string id;
    std::string status;
    std::string main_source;
    std::string in_multicast;
    std::string out_multicast;
};

int loadConfigFromJson(const std::string &filename, Config &config);

std::vector<ColorRange> loadColorRangesFromJson(const std::string &filename);

void readDataFromFile(const std::string &filename, std::vector<StreamData> &streamDataVector);

int streamsJsonMaker(const std::vector<StreamData> &streamDataVector);

void writeStreamsToJson(const std::string &filename, const Streams &streams);

int readStreamsFromJson(const std::string &filename, Streams &streams);


#endif //STREAM_ANALYSIS_CONFIG_HPP
