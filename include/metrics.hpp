//
// Created by gio on 10/23/24.
//

#ifndef STREAM_ANALYSIS_METRICS_HPP
#define STREAM_ANALYSIS_METRICS_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Metrics {
    int black_frame_count;
    int static_frame_count;
    int blank_frame_count;
    bool coloured_stripes_detected;
    bool no_input_stream;
    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, black_frame_count, static_frame_count, blank_frame_count,
                                   coloured_stripes_detected, no_input_stream)
};


int writeResultsToJson(const std::string &filename, const Metrics &metrics);

void printMetrics(const Metrics &metrics);

std::string getTimeString();

int getLogCount(const std::string &filename);

#endif //STREAM_ANALYSIS_METRICS_HPP
