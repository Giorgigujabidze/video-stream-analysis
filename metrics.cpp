#include "metrics.hpp"
#include "fstream"
#include "iostream"

void writeResultsToCSV(const std::string &filename, Metrics &metrics) {
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%F %T", localtime(&now));
    //std::vector<Metrics> results = loadResultsLog(filename);
    metrics.time = buf;


    const std::string content = metrics.time + ",  " + std::to_string(metrics.blankFrameCount) + ",  " +
                          std::to_string(metrics.staticFrameCount) +
                          ",  " +
                          std::to_string(metrics.blackFrameCount) + ",  " + std::to_string(metrics.corruptFrameCount) +
                          ",  " +
                          std::to_string(metrics.colouredStripesDetected) + "\n";

    filePutContents(filename, content, true);
}


void printMetrics(const Metrics &metrics) {
    std::cout << "Time: " << metrics.time << std::endl
              << "Colored Stripes Detected: " << metrics.colouredStripesDetected << std::endl
              << "Black Frames: " << metrics.blackFrameCount << std::endl
              << "Static Frames: " << metrics.staticFrameCount << std::endl
              << "Blank Frames: " << metrics.blankFrameCount << std::endl
              << "Corrupt Frames: " << metrics.corruptFrameCount << std::endl;
}

void filePutContents(const std::string &filename, const std::string &content, bool append = false) {
    std::ofstream outfile;
    if (append) {
        outfile.open(filename, std::ios_base::app);
    } else {
        outfile.open(filename, std::ios_base::trunc);
    }
    outfile << content;
}
