//
// Created by gio on 10/23/24.
//

#ifndef STREAM_ANALYSIS_METRICS_HPP
#define STREAM_ANALYSIS_METRICS_HPP

#include <string>
#include <vector>
#include <nlohmann/json.hpp>

struct Metrics {
    std::string time;
    bool colouredStripesDetected;
    int blackFrameCount;
    int staticFrameCount;
    int blankFrameCount;
};

void writeResultsToCSV(const std::string &filename, Metrics &metrics, int maxLogNumber);

void printMetrics(const Metrics &metrics);

void filePutContents(const std::string &filename, const std::string &content, bool append);

std::string getTimeString();

int getLogCount(const std::string &filename);

#endif //STREAM_ANALYSIS_METRICS_HPP
