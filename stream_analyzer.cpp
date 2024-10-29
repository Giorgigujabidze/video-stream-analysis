#include "stream_analyzer.hpp"
#include "frame_analysis.hpp"

void analyzeVideoStream(cv::VideoCapture &cap, const Config &config, const std::vector<ColorRange> &colorRanges,
                        Metrics &metrics, std::vector<double> &buffer1) {
    cv::Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    const auto start = std::chrono::high_resolution_clock::now();
    int frameCounter = 0;
    while (true) {
        if (metrics.blankFrameCount > 10) {
            break;
        }

        if (!cap.read(frame)) {
            std::cerr << "Blank frame grabbed\n";
            metrics.blankFrameCount++;
            continue;
        }

        resize(frame, downscaledFrame, cv::Size(600, 400), 0, 0, cv::INTER_LINEAR);
        cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);

        if (prevGrayFrame.empty()) {
            prevGrayFrame = grayFrame.clone();
            continue;
        }


        if (detectStaticFrame(grayFrame, prevGrayFrame, config.thresholds.staticFrameThreshold, buffer1,
                              config.sizeParameters.maxMeanBufferSize)) {
            metrics.staticFrameCount++;
            if (detectBlackFrame(grayFrame, config.thresholds.blackFrameThreshold)) {
                metrics.blackFrameCount++;
            } else if (!metrics.colouredStripesDetected &&
                       detectColouredStripes(downscaledFrame, colorRanges, config.thresholds.colouredStripesThreshold,
                                             config.thresholds.colouredStripesMaxDeviation)) {
                metrics.colouredStripesDetected = true;
            }
        }
        prevGrayFrame = grayFrame.clone();
        auto now = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start);
        frameCounter++;
        if (duration.count() >= config.interval) {
            break;
        }
    }
}
