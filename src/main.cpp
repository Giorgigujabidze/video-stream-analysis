#include <atomic>

#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <pthread.h>

#include "ffmpeg_capture.hpp"
#include "threading.hpp"
#include "helpers.hpp"
#include <csignal>

auto gStopFlag = std::make_shared<std::atomic<bool> >(false);

void signalHandler(int signum) {
    gStopFlag->store(true);
}

int main(const int argc, char **argv) {
    std::signal(SIGINT, signalHandler);
    std::signal(SIGTERM, signalHandler);
    std::signal(SIGABRT, signalHandler);

    if (argc > 3) {
        getHelp(argv[0]);
        return -1;
    }

    programSetup();

    int n = 0;

    if (argc == 2 && std::string_view(argv[1]) == "--help") {
        getHelp(argv[0]);
        return -1;
    }

    if (argc == 3 && std::string_view(argv[1]) == "-c") {
        if (startStreamsFileMaker(argv[2]) < 0) {
            return -1;
        }
        return 0;
    }

    if (argc == 3 && std::string_view(argv[1]) == "-n") {
        n = atoi(argv[2]);
    }

    const auto config = new Config{};
    if (loadConfigFromJson("../program_config/program_config.json", *config) < 0) {
        std::cerr << "failed to load config from json" << std::endl;
        return -1;
    }
    validateProgramConfig(*config);

    auto streams = Streams{};
    if (readStreamsFromJson("../streams/streams.json", streams) < 0) {
        log(*config, "failed to load streams", ERROR);
        return -1;
    }

    const std::vector<ColorRange> colorRanges = loadColorRangesFromJson("../color_ranges/color_ranges.json");
    if (colorRanges.empty()) {
        log(*config, "failed to load color ranges", ERROR);
        return -1;
    }

    if (n == 0 || n > streams.streams.size()) {
        n = streams.streams.size();
    }

    std::vector<pthread_t> threads;
    threads.reserve(n);

    constexpr int BATCH_SIZE = 20;

    for (int i = 0; i < n; i += BATCH_SIZE) {
        const int batchEnd = std::min(i + BATCH_SIZE, n);
        for (int j = i; j < batchEnd; j++) {
            const auto threadArgs = new ThreadArguments{
                .config = *config,
                .stream = streams.streams[j],
                .colorRanges = colorRanges,
                .stopFlag = gStopFlag
            };
            pthread_t thread;
            if (pthread_create(&thread, nullptr, analyzeVideoStream, threadArgs) != 0) {
                log(*config, "failed to create thread for stream: " + streams.streams[j].name, ERROR);
                delete threadArgs;
            } else {
                threads.push_back(thread);
            }
        }
        sleep(2);
    }

    if (threads.empty()) {
        log(*config, "failed to create threads", ERROR);
        delete config;
        return -1;
    }

    std::cout << "starting to grab frames\n";
    for (const auto &thread: threads) {
        pthread_join(thread, nullptr);
    }

    delete config;

    return 0;
}
