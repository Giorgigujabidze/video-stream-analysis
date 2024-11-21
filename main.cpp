#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <pthread.h>
#include "threading.hpp"


int main(const int argc, char **argv) {
    if (argc > 1 && std::string_view(argv[1]) == "--help") {
        std::cout << "usages: " << std::endl
                << argv[0] << std::endl
                << argv[0] << " <-c> <data_file>" << std::endl;
        return -1;
    }

    if (argc > 1 && std::string_view(argv[1]) == "-c") {
        std::vector<StreamData> streamDataVector;
        readDataFromFile(argv[2], streamDataVector);
        if (configMaker("../config/config.json", streamDataVector) < 0) {
            std::cerr << "failed to read sample config file\n";
            return -1;
        }
        std::cout << "configs where successfully generated\n";
        return 0;
    }

    auto configs = Configs{};
    if (readConfigsFromJson("../config/configs.json", configs) < 0) {
        return -1;
    }

    const std::vector<ColorRange> colorRanges = loadColorRangesFromJson("../color_ranges/color_ranges.json");
    if (colorRanges.empty()) {
        std::cout << "failed to load color ranges\n";
        return -1;
    }

    std::vector threads(configs.configs.size(), pthread_t{});

    std::cout << "starting to grab frames\n";
    for (int i = 0; i < configs.configs.size(); i++) {
        auto threadArgs = new ThreadArguments{
            .config = configs.configs[i], .colorRanges = colorRanges,
        };
        if (pthread_create(&threads[i], nullptr, analyzeVideoStream, threadArgs) != 0) {
            std::cerr << "failed to create thread for config: " << configs.configs[i].name << std::endl;
            delete threadArgs;
        }
    }
    for (const auto &thread: threads) {
        pthread_join(thread, nullptr);
    }
    return 0;
}
