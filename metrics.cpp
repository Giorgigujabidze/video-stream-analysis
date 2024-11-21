#include "metrics.hpp"
#include "fstream"
#include "iostream"




void printMetrics(const Metrics &metrics) {
    std::cout << "Time: " << getTimeString() << std::endl
            << "No Input Stream: " << metrics.no_input_stream << std::endl
            << "Colored Stripes Detected: " << metrics.coloured_stripes_detected << std::endl
            << "Black Frames: " << metrics.black_frame_count << std::endl
            << "Static Frames: " << metrics.static_frame_count << std::endl
            << "Blank Frames: " << metrics.blank_frame_count << std::endl;
}

void writeResultsToJson(const std::string &filename,const Metrics &metrics) {
    nlohmann::json j;
    std::ofstream outfile(filename);

    j = metrics;

    outfile << j.dump(4);
}

void filePutContents(const std::string &filename, const std::string &content, const bool append = false) {
    std::ofstream outfile;
    if (append) {
        outfile.open(filename, std::ios_base::app);
    } else {
        outfile.open(filename, std::ios_base::trunc);
    }
    outfile << content;
}


std::string getTimeString() {
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%F %T", localtime(&now));
    return buf;
}
