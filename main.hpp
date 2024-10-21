//
// Created by gio on 10/19/24.
//

#ifndef UNTITLED8_MAIN_HPP
#define UNTITLED8_MAIN_HPP

#include <iostream>
#include <opencv4/opencv2/opencv.hpp>
#include <nlohmann/json.hpp>
#include <fstream>
#include <chrono>
#include <thread>
#include <opencv2/core/utils/logger.hpp>


using namespace cv;
using namespace std;
using json = nlohmann::json;


struct Thresholds {
    double staticFrameThreshold;
    double colouredStripesThreshold;
    double colouredStripesMaxDeviation;
    double blackFrameThreshold;
    double artifactDetectionThreshold;

};

struct SizeParameters {
    int maxMeanBufferSize;
    int maxCorrelationBufferSize;
    int histogramSize;
};

struct Config {
    string url;
    string colorRangesPath;
    string corruptedFramesPath;
    Thresholds thresholds;
    SizeParameters sizeParameters;
    time_t interval;
    bool keyframesOnly;
};

struct Metrics {
    string time;
    bool colouredStripesDetected;
    int blackFrameCount;
    int staticFrameCount;
    int blankFrameCount;
    int corruptFrameCount;

    NLOHMANN_DEFINE_TYPE_INTRUSIVE(Metrics, time, colouredStripesDetected, blackFrameCount, staticFrameCount,
                                   blankFrameCount,
                                   corruptFrameCount);
};


struct ColorRange {
    string name;
    Scalar lower;
    Scalar upper;
};

bool isKeyFrame(const Mat &frame, const Mat &prevFrame, double &threshold, const float **histRange, const int &histSize,
           int &frameCounter, int forcedKeyframeInterval = 30);

bool detectBlackFrame(const Mat &frame, const double &threshold);

bool detectStaticFrame(const Mat &frame, const Mat &prevFrame, const double &threshold, vector<double> &buffer,
                       int maxBufferSize);

bool detectColouredStripes(const Mat &frame, const vector<ColorRange> &colorRanges, const double &threshold1,
                           const double &threshold2);

bool detectArtifacts(const Mat &frame, const Mat &prevFrame, vector<double> &buffer, const string &filename,
                     const double &threshold, int max_buffer_size, const float **histRange, const int &histSize);

vector<ColorRange> loadColorRangesFromJson(const string &filename);

void writeResultsToJson(const string &filename, Metrics &metrics);

void loadConfigFromJson(const string &filename, Config &config);

vector<Metrics> loadResultsLog(const string &filename);

void analyzeVideoStream(VideoCapture &cap, const Config &config, const vector<ColorRange> &colorRanges,
                        Metrics &metrics, vector<double> &buffer1, vector<double> &buffer2, const float **histRange);

void filePutContents(const string &filename, const json &content, bool append);

void printMetrics(const Metrics &metrics);

#endif //UNTITLED8_MAIN_HPP
