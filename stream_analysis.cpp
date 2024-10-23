#include "stream_analysis.hpp"


int main(int argc, char **argv) {
    if (argc < 3) {
        cerr << "You should specify config file and result file paths";
        return -1;
    }

    Config config = Config{};
    if (loadConfigFromJson(argv[1], config) < 0) {
        cout << "failed to load config file\n";
        return -1;
    }

    vector<ColorRange> colorRanges = loadColorRangesFromJson(config.colorRangesPath);
    if (colorRanges.empty()) {
        cout << "failed to load color ranges\n";
        return -1;
    }
    vector<double> meanBuffer = {};
    vector<double> correlationBuffer = {};
    VideoCapture cap;

    float range[] = {0, static_cast<float>(config.sizeParameters.histogramSize)};
    const float *histRange = {range};

    if (!cap.open(config.url, CAP_FFMPEG, {CAP_PROP_HW_ACCELERATION, VIDEO_ACCELERATION_VAAPI})) {
        cerr << "Failed to open video stream\n";
        return -1;
    }
    //cv::utils::logging::setLogLevel(cv::utils::logging::LOG_LEVEL_VERBOSE);

    cout << "Starting to grab frames\n";
    while (true) {
        auto metrics = Metrics{};
        analyzeVideoStream(cap, config, colorRanges, metrics, meanBuffer, correlationBuffer, &histRange);
        printMetrics(metrics);
        writeResultsToJson(argv[2], metrics);
        this_thread::sleep_for(chrono::seconds(3));
    }

    return 0;
}

void analyzeVideoStream(VideoCapture &cap, const Config &config, const vector<ColorRange> &colorRanges,
                        Metrics &metrics, vector<double> &buffer1, vector<double> &buffer2, const float **histRange) {
    Mat frame, downscaledFrame, prevGrayFrame, grayFrame;
    auto start = chrono::high_resolution_clock::now();
    int frameCounter = 0;
    int passedFrameCounter = 0;
    double threshold = 0;
    while (true) {
        if (metrics.blankFrameCount > 10) {
            break;
        }

        if (!cap.read(frame)) {
            cerr << "Blank frame grabbed\n";
            metrics.blankFrameCount++;
            continue;
        }

        resize(frame, downscaledFrame, Size(600, 400), INTER_AREA);
        cvtColor(downscaledFrame, grayFrame, COLOR_BGR2GRAY);

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

        string filename = "frame" + to_string(frameCounter) + ".pgm";
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
        auto now = chrono::high_resolution_clock::now();
        auto duration = chrono::duration_cast<chrono::seconds>(now - start);
        frameCounter++;
        //imshow("window", downscaledFrame);
        if (/*waitKey(config.interval) >= 0 ||*/ duration.count() >= config.interval) {
            //destroyAllWindows();
            break;
        }
    }
}


bool isKeyFrame(const Mat &frame, const Mat &prevFrame, double &threshold, const float **histRange, const int &histSize,
                int &frameCounter, int forcedKeyframeInterval) {
    Mat histOne, histTwo;

    calcHist(&frame, 1, nullptr, Mat(), histOne, 1, &histSize, histRange);
    calcHist(&prevFrame, 1, nullptr, Mat(), histTwo, 1, &histSize, histRange);

    normalize(histOne, histOne, 0, 1, NORM_MINMAX);
    normalize(histTwo, histTwo, 0, 1, NORM_MINMAX);

    Mat diff;
    absdiff(histOne, histTwo, diff);

    double diffMean = mean(diff)[0];

    if (diffMean < threshold) {
        cout << "not a key frame!\n";
    } else {
        cout << "**key frame!**\n";
    }
    //cout << "Difference mean: " << diffMean << "\n";
    //cout << "Current threshold: " << threshold << "\n";

    if (threshold == 0) {
        Scalar meanVal, stdDevVal;
        meanStdDev(diff, meanVal, stdDevVal);
        threshold = meanVal[0] + stdDevVal[0];
        cout << "New threshold set: " << threshold << "\n";
    }

    if (frameCounter && (frameCounter % forcedKeyframeInterval == 0)) {
        cout << "Forcing keyframe due to frame interval.\n";
        frameCounter = 0;
        return true;
    }

    if (diffMean > threshold) {
        frameCounter = 0;
    }

    return diffMean > threshold;
}


bool detectArtifacts(const Mat &frame, const Mat &prevFrame, vector<double> &buffer, const string &filename,
                     const double &threshold, int max_buffer_size, const float **histRange, const int &histSize) {
    Mat histOne, histTwo;

    calcHist(&frame, 1, nullptr, Mat(), histOne, 1, &histSize, histRange);
    calcHist(&prevFrame, 1, nullptr, Mat(), histTwo, 1, &histSize, histRange);

    normalize(histOne, histOne, 0, 1, NORM_MINMAX, -1, Mat());
    normalize(histTwo, histTwo, 0, 1, NORM_MINMAX, -1, Mat());

    double correlation = compareHist(histOne, histTwo, HISTCMP_CORREL);

    if (buffer.size() < max_buffer_size) {
        buffer.push_back(correlation);
        return false;
    }
    double correlationAverage = mean(buffer)[0];
    // cout << "Histogram correlation: " << correlationAverage << "\n";
    if (correlationAverage < threshold) {
        imwrite("prev" + filename, prevFrame);
        imwrite(filename, frame);
        cout << "Possible frame corruption detected due to low histogram correlation!\n";
    }
    buffer.clear();
    // cout << "Frames are similar, no corruption detected.\n";
    return correlationAverage < threshold;
}

bool detectColouredStripes(const Mat &frame, const vector<ColorRange> &colorRanges, const double &threshold1,
                           const double &threshold2) {
    Mat hsvFrame;
    cvtColor(frame, hsvFrame, COLOR_BGR2HSV);
    Mat combinedMask = Mat::zeros(frame.size(), CV_8U);
    vector<double> colorDistributions = {};

    for (const auto &range: colorRanges) {
        Mat mask;
        inRange(hsvFrame, range.lower, range.upper, mask);
        combinedMask |= mask;
        colorDistributions.push_back((mean(mask)[0] / 255) * 100);
    }

    Scalar meanVal, stdDevVal;
    meanStdDev(colorDistributions, meanVal, stdDevVal);

    double scalingFactor = 100.0 / colorRanges.size();
    double colouredStripesProbability = (meanVal[0] / scalingFactor) * 100;

    cout << "combinedMask: " << colouredStripesProbability << "%\n";

    if (colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2) {
        imshow("coloured stripes", combinedMask);


    }
    return colouredStripesProbability > threshold1 && stdDevVal[0] < threshold2;
}

bool detectBlackFrame(const Mat &frame, const double &threshold) {
    cout << "mean: " << mean(frame)[0] << "\n";
    return mean(frame)[0] < threshold;
}

bool detectStaticFrame(const Mat &frame, const Mat &prevFrame, const double &threshold, vector<double> &buffer,
                       const int maxBufferSize) {
    Mat diff;
    absdiff(frame, prevFrame, diff);
    double bufferAverage = 0;
    double avgDifference = mean(diff)[0];
    if (buffer.size() < maxBufferSize) {
        buffer.push_back(avgDifference);
        return false;
    }
    bufferAverage = mean(buffer)[0];
    cout << bufferAverage << '\n';
    buffer.clear();
    return bufferAverage < threshold;
}

vector<ColorRange> loadColorRangesFromJson(const string &filename) {
    ifstream file(filename);
    json j;
    vector<ColorRange> colorRanges;

    try {
        file >> j;
    } catch (json::parse_error &e) {
        cout << e.what() << '\n';
        return colorRanges;
    }

    for (auto &range: j["colorRanges"]) {
        colorRanges.push_back({
                                      range["name"],
                                      Scalar(range["lower"]["h"], range["lower"]["s"], range["lower"]["v"]),
                                      Scalar(range["upper"]["h"], range["upper"]["s"], range["upper"]["v"])
                              });
    }
    return colorRanges;
}

void writeResultsToJson(const string &filename, Metrics &metrics) {
    json j;
    time_t now;
    time(&now);
    char buf[sizeof "2011-10-08T07:07:09Z"];
    strftime(buf, sizeof buf, "%F %T", localtime(&now));
    vector<Metrics> results = loadResultsLog(filename);
    metrics.time = buf;
    results.push_back(metrics);
    j["results"] = results;

    filePutContents(filename, j, false);
}


vector<Metrics> loadResultsLog(const string &filename) {
    ifstream file(filename);
    json j;
    vector<Metrics> results = {};
    try {
        file >> j;
    } catch (json::parse_error &e) {
        cerr << e.what() << "\n";
        return results;
    }

    if (j.empty()) {
        return results;
    }

    if (j["results"].size() > 100) {
        return results;
    }

    for (auto &result: j["results"]) {
        results.push_back({result["time"],
                           result["colouredStripesDetected"],
                           result["blackFrameCount"],
                           result["staticFrameCount"],
                           result["blankFrameCount"],
                           result["corruptFrameCount"]});
    }

    return results;

}


int loadConfigFromJson(const string &filename, Config &config) {
    ifstream file(filename);
    json j;

    try {
        file >> j;
    } catch (json::parse_error &e) {
        cout << e.what();
        return -1;
    }

    config.url = j["url"];
    config.colorRangesPath = j["color_ranges_path"];
    config.corruptedFramesPath = j["corrupted_frames_path"];
    config.keyframesOnly = j["keyframes_only"];
    config.interval = j["interval"];
    config.thresholds = {
            j["thresholds"]["static_frame_threshold"],
            j["thresholds"]["coloured_stripes_threshold"],
            j["thresholds"]["coloured_stripes_max_deviation"],
            j["thresholds"]["black_frame_threshold"],
            j["thresholds"]["artifact_detection_threshold"],
    };
    config.sizeParameters = {
            j["size_parameters"]["max_mean_buffer_size"],
            j["size_parameters"]["max_correlation_buffer_size"],
            j["size_parameters"]["histogram_size"],
    };
    return 0;
}

void filePutContents(const string &filename, const json &content, bool append = false) {
    ofstream outfile;
    if (append) {
        outfile.open(filename, ios_base::app);
    } else {
        outfile.open(filename, ios_base::trunc);
    }
    outfile << content.dump(4);
}

void printMetrics(const Metrics &metrics) {
    cout << "blank frames: " << metrics.blankFrameCount << " |***| " << "black frames: " << metrics.blackFrameCount
         << " |***| "
         << "coloured stripes: " << metrics.colouredStripesDetected << " |***| "
         << "static frame count: " << metrics.staticFrameCount << " |***| " << "corrupt frame count: "
         << metrics.corruptFrameCount << '\n';
}