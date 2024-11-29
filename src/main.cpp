#include "stream_analyzer.hpp"
#include "config.hpp"
#include "metrics.hpp"
#include <iostream>
#include <pthread.h>

#include "frame.hpp"
#include "threading.hpp"
#include "helpers.hpp"


int main(const int argc, char **argv) {
    if (argc > 3) {
        getHelp(argv[0]);
        return -1;
    }

    programSetup();

    int n = 0;

 /*
    Capture cap;
    if (cap.openStream("https://edge01.tbs.nucast.tv/gpb-2tv/tracks-v1a1/mono.m3u8") < 0) {
        std::cerr << "Failed to open stream" << std::endl;
        return -1;
    }
    cv::Mat frame, downscaledFrame, grayFrame, prevGrayFrame;
    std::vector<double> meanBuffer;

    while (true) {
        if (cap.grabFrame() < 0) {
            continue;
        }

        if (cap.retrieveFrame() != DECODE_OK) {
            continue;
        }


        if (frame.empty()) {
            std::cerr << "Empty frame" << std::endl;
            continue;
        }

        preprocessFrame(frame, downscaledFrame, grayFrame);

        if (prevGrayFrame.empty()) {
            prevGrayFrame = grayFrame.clone();
            continue;;
        }
        detectStaticFrame(grayFrame, prevGrayFrame, 0.03, meanBuffer, 15);
        detectBlackFrame(grayFrame, 15);

        prevGrayFrame = grayFrame.clone();

        cv::imshow("frame", downscaledFrame);
        cv::waitKey(15);
        frame.release();
    }
*/


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
        return -1;
    }

    auto streams = Streams{};
    if (readStreamsFromJson("../streams/streams.json", streams) < 0) {
        return -1;
    }

    const std::vector<ColorRange> colorRanges = loadColorRangesFromJson("../color_ranges/color_ranges.json");
    if (colorRanges.empty()) {
        std::cout << "failed to load color ranges\n";
        return -1;
    }

    std::vector threads(streams.streams.size(), pthread_t{});

    if (n == 0 || n > streams.streams.size()) {
        n = streams.streams.size();
    }

    for (int i = 0; i < n; i++) {
        const auto threadArgs = new ThreadArguments{
            .config = *config, .stream = streams.streams[i],
            .colorRanges = colorRanges
        };
        if (pthread_create(&threads[i], nullptr, analyzeVideoStream, threadArgs) != 0) {
            std::cerr << "failed to create thread for streams: " << streams.streams[i].name << std::endl;
            delete threadArgs;
        }
    }
    std::cout << "starting to grab frames\n";
    for (const auto &thread: threads) {
        pthread_join(thread, nullptr);
    }

    return 0;
}
