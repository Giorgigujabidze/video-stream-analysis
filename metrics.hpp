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
    int corruptFrameCount;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, time, colouredStripesDetected,
                                   blackFrameCount, staticFrameCount,
                                   blankFrameCount, corruptFrameCount);
};

void writeResultsToJson(const std::string& filename, Metrics& metrics);
std::vector<Metrics> loadResultsLog(const std::string& filename);
void printMetrics(const Metrics& metrics);
void filePutContents(const std::string& filename, const nlohmann::json& content, bool append);



#endif //STREAM_ANALYSIS_METRICS_HPP
