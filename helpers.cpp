#include "helpers.hpp"

#include <fstream>
#include <opencv2/core/utils/logger.hpp>

#include "config.hpp"

void programSetup() {
    setenv("OPENCV_FFMPEG_IS_THREAD_SAFE", "1", 0);
    setenv("OPENCV_FFMPEG_LOGLEVEL", "-8", 0);
    setLogLevel(cv::utils::logging::LOG_LEVEL_SILENT);
    filePutContents("../failed_streams/failed_streams.txt", "", false);
}

void getHelp(const std::string &name) {

    std::cout << "usages: " << std::endl
            << name << std::endl
            << name << " <-c> <data_file>" << std::endl;
}

int startConfigMaker(const std::string &filename) {

    std::vector<StreamData> streamDataVector;
    readDataFromFile(filename, streamDataVector);

    if (configMaker("../sample_config/config.json", streamDataVector) < 0) {
        std::cerr << "failed to read sample config file\n";
        return -1;
    }

    std::cout << "configs where successfully generated\n";
    return 0;
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

