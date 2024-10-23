#include "stream_analyzer.hpp"
#include "frame_analysis.hpp"

void analyzeVideoStream(cv::VideoCapture &cap, const Config &config, const std::vector<ColorRange> &colorRanges,
                        Metrics &metrics, std::vector<double> &buffer1,std::vector<double> &buffer2, const float **histRange) {
    cv::Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    auto start = std::chrono::high_resolution_clock::now();
    int frameCounter = 0;
    int passedFrameCounter = 0;
    double threshold = 0;
    while (true) {
        if (metrics.blankFrameCount > 10) {
            break;
        }

        if (!cap.read(frame)) {
            std::cerr << "Blank frame grabbed\n";
            metrics.blankFrameCount++;
            continue;
        }

        resize(frame, downscaledFrame, cv::Size(600, 400), cv::INTER_AREA);
        cvtColor(downscaledFrame, grayFrame, cv::COLOR_BGR2GRAY);

        if (prevGrayFrame.empty()) {
            prevGrayFrame = grayFrame.clone();
            continue;
        }

        if (config.keyframesOnly && !isKeyFrame(grayFrame, prevGrayFrame, threshold, histRange,
                                                config.sizeParameters.histogramSize, passedFrameCounter)) {
            prevGrayFrame = grayFrame.clone();
            passedFrameCounter++;
            continue;
        }

        std::string filename = "frame" + std::to_string(frameCounter) + ".pgm";
        if (detectArtifacts(grayFrame, prevGrayFrame, buffer2, config.corruptedFramesPath + filename,
                            config.thresholds.artifactDetectionThreshold,
                            config.sizeParameters.maxCorrelationBufferSize,
                            histRange,
                            config.sizeParameters.histogramSize)) {
            metrics.corruptFrameCount++;
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
        //imshow("window", downscaledFrame);
        if (/*waitKey(config.interval) >= 0 ||*/ duration.count() >= config.interval) {
            //destroyAllWindows();
            break;
        }
    }
}
