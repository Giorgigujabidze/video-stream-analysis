#include "config.hpp"
#include <fstream>
#include <iostream>
#include <unordered_set>


int loadConfigFromJson(const std::string &filename, Config &config) {
    std::ifstream file(filename);
    nlohmann::json j;

    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cerr << e.what();
        return -1;
    }

    config = j;

    return 0;
}

std::vector<ColorRange> loadColorRangesFromJson(const std::string &filename) {
    std::ifstream file(filename);
    nlohmann::json j;
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

void readDataFromFile(const std::string &filename, std::vector<StreamData> &streamDataVector
) {
    std::ifstream file(filename);
    std::string line, token;
    StreamData streamData;
    std::vector<std::string> tokenVector;
    std::getline(file, line);

    //std::cout << line << std::endl;

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        while (std::getline(ss, token, ',')) {
            if (token[0] == '"') {
                std::getline(ss, token, ',');
            }
            tokenVector.push_back(token);
        }
        // std::cout << std::endl;
        streamData.lid = tokenVector[0];
        streamData.name = tokenVector[1];
        streamData.id = tokenVector[2];
        streamData.status = tokenVector[3];
        streamData.main_source = tokenVector[4];
        streamData.in_multicast = tokenVector[5];
        streamData.out_multicast = tokenVector[6];
        //std::cout << tokenVector[4] << "----" << tokenVector[5] << "----" << tokenVector[6] << '\n';

        tokenVector.clear();
        streamDataVector.push_back(streamData);
    }

    /* for (auto &data: streamDataVector) {
         std::cout << data.name << std::endl;
     }*/
}

void configMaker(const std::string &filename, const std::vector<StreamData> &streamDataVector) {
    Config config = {};
    loadConfigFromJson(filename, config);
    std::string prevLid;
    std::pmr::unordered_set<std::string> multicastSet;
    for (auto &data: streamDataVector) {
        if (data.status == "NO") {
            continue;
        }
        std::string urlIn = "udp://" + data.in_multicast;
        std::string urlOut = "udp://" + data.out_multicast;
        /*if (multicastSet.contains(urlIn)) {
            std::cout << urlIn << std::endl;
        }*/
       if (!multicastSet.contains(urlIn)) {
            writeConfig(urlIn, config, data, "_in_");
            //std::cout << urlIn << std::endl;
        }

        multicastSet.insert(urlIn);

        writeConfig(urlOut, config, data, "_out_");
    }
}

void writeConfig(const std::string &url, Config &config, const StreamData &data, const std::string &flag) {
    nlohmann::json j;
    config.url = url;
    j = config;
    std::ofstream file("../config/config" + flag + data.id + ".json");
    //std::cout << j["url"] << '\n';
    file << j.dump();
}
