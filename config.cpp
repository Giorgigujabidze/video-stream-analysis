#include "config.hpp"
#include <fstream>
#include <iostream>
#include <set>

#include "metrics.hpp"


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

    while (std::getline(file, line)) {
        std::stringstream ss(line);
        while (std::getline(ss, token, ',')) {
            if (token[0] == '"') {
                std::getline(ss, token, ',');
            }
            tokenVector.push_back(token);
        }

        streamData.lid = tokenVector[0];
        streamData.name = tokenVector[1];
        streamData.id = tokenVector[2];
        streamData.status = tokenVector[3];
        streamData.main_source = tokenVector[4];
        streamData.in_multicast = tokenVector[5];
        streamData.out_multicast = tokenVector[6];

        tokenVector.clear();
        streamDataVector.push_back(streamData);
    }
}

int configMaker(const std::string &filename, const std::vector<StreamData> &streamDataVector) {
    Config config = {};
    Configs configs = {};
    if (loadConfigFromJson(filename, config) < 0) {
        return -1;
    }
    std::string prevLid;
    std::set<std::string> multicastSet;
    for (auto &data: streamDataVector) {
        if (data.status == "NO") {
            continue;
        }
        std::string urlIn = "udp://" + data.in_multicast;
        std::string urlOut = "udp://" + data.out_multicast;

        if (!multicastSet.contains(urlIn)) {
            config.name = "_in_" + data.id;
            config.url = urlIn;
            configs.configs.push_back(config);
        }

        multicastSet.insert(urlIn);

        config.name = "_out_" + data.id;
        config.url = urlOut;
        configs.configs.push_back(config);
    }
    writeConfigToJson("../config/configs.json", configs);
    return 0;
}


void writeConfigToJson(const std::string &filename, const Configs &configs) {
    std::ofstream file(filename);
    nlohmann::json j;
    j = configs;
    file << j.dump(4);
}

int readConfigsFromJson(const std::string &filename, Configs &configs) {
    std::ifstream file(filename);
    nlohmann::json j;
    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cout << e.what() << '\n';
        return -1;
    }
    configs = j;
    return 0;
}
