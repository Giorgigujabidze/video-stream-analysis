#include "metrics.hpp"
#include "fstream"
#include "iostream"

void writeResultsToJson(const std::string &filename, Metrics &metrics) {
    nlohmann::json j;
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%F %T", localtime(&now));
    std::vector<Metrics> results = loadResultsLog(filename);
    metrics.time = buf;
    results.push_back(metrics);
    j["results"] = results;

    filePutContents(filename, j, false);
}

std::vector<Metrics> loadResultsLog(const std::string &filename) {
    std::ifstream file(filename);
    nlohmann::json j;
    std::vector<Metrics> results = {};
    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cerr << e.what() << "\n";
        return results;
    }

    if (j.empty()) {
        return results;
    }

    if (j["results"].size() > 100) {
        return results;
    }

    for (auto &result: j["results"]) {
        results.push_back({result["time"],
                           result["colouredStripesDetected"],
                           result["blackFrameCount"],
                           result["staticFrameCount"],
                           result["blankFrameCount"],
                           result["corruptFrameCount"]});
    }

    return results;

}


void printMetrics(const Metrics &metrics) {
    std::cout << "Time: " << metrics.time << std::endl
              << "Colored Stripes Detected: " << metrics.colouredStripesDetected << std::endl
              << "Black Frames: " << metrics.blackFrameCount << std::endl
              << "Static Frames: " << metrics.staticFrameCount << std::endl
              << "Blank Frames: " << metrics.blankFrameCount << std::endl
              << "Corrupt Frames: " << metrics.corruptFrameCount << std::endl;
}

void filePutContents(const std::string &filename, const nlohmann::json &content, bool append = false) {
    std::ofstream outfile;
    if (append) {
        outfile.open(filename, std::ios_base::app);
    } else {
        outfile.open(filename, std::ios_base::trunc);
    }
    outfile << content.dump(4);
}
