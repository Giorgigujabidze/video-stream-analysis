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

int readDataFromFile(const std::string &filename, std::vector<StreamData> &streamDataVector
) {
    std::ifstream file(filename);

    if (!file.is_open()) {
        std::cerr << "file " << filename << " does not exist." << std::endl;
        return -1;
    }

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

        streamData = {
            .lid = tokenVector[0], .name = tokenVector[1], .id = tokenVector[2], .status = tokenVector[3],
            .main_source = tokenVector[4], .in_multicast = tokenVector[5], .out_multicast = tokenVector[6]
        };

        tokenVector.clear();
        streamDataVector.push_back(streamData);
    }
    return 0;
}

int streamsJsonMaker(const std::vector<StreamData> &streamDataVector) {
    Stream stream = {};
    Streams streams = {};

    std::string prevLid;
    std::set<std::string> multicastSet;

    for (auto &data: streamDataVector) {
        if (data.status == "NO") {
            continue;
        }

        const std::string urlIn = "udp://" + data.in_multicast;
        const std::string urlOut = "udp://" + data.out_multicast;

        if (!multicastSet.contains(urlIn)) {
            stream.name = "_in_" + data.id;
            stream.url = urlIn;
            streams.streams.push_back(stream);
        }

        multicastSet.insert(urlIn);

        stream.name = "_out_" + data.id;
        stream.url = urlOut;
        streams.streams.push_back(stream);
    }
    writeStreamsToJson("../streams/streams.json", streams);
    return 0;
}


void writeStreamsToJson(const std::string &filename, const Streams &streams) {
    std::ofstream file(filename);
    nlohmann::json j;
    j = streams;
    file << j.dump(4);
}

int readStreamsFromJson(const std::string &filename, Streams &streams) {
    std::ifstream file(filename);
    nlohmann::json j;
    try {
        file >> j;
    } catch (nlohmann::json::parse_error &e) {
        std::cout << e.what() << '\n';
        return -1;
    }
    streams = j;
    return 0;
}
