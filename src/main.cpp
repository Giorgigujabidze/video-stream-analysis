#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <pthread.h>
#include "threading.hpp"
#include "helpers.hpp"

int main(const int argc, char **argv) {
    programSetup();

    int n = 0;

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        getHelp(argv[0]);
        return -1;
    }

    if (argc == 3 && std::string_view(argv[1]) == "-c") {
        if (startConfigMaker(argv[2]) < 0) {
            return -1;
        }
        return 0;
    }

    if (argc == 3 && std::string_view(argv[1]) == "-n") {
        n = atoi(argv[2]);
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

    if (n == 0) {
        n = configs.configs.size();
    }

    for (int i = 0; i < n; i++) {
        auto threadArgs = new ThreadArguments{
            .config = configs.configs[i], .colorRanges = colorRanges
        };
        if (pthread_create(&threads[i], nullptr, analyzeVideoStream, threadArgs) != 0) {
            std::cerr << "failed to create thread for config: " << configs.configs[i].name << std::endl;
            delete threadArgs;
        }
    }
    std::cout << "starting to grab frames\n";
    for (const auto &thread: threads) {
        pthread_join(thread, nullptr);
    }

    return 0;
}
