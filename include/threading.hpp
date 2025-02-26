//
// Created by gio on 11/19/24.
//

#ifndef THREADING_HPP
#define THREADING_HPP

#include <opencv2/opencv.hpp>
#include "config.hpp"

struct ThreadArguments {
    Config config;
    Stream stream;
    std::vector<ColorRange> colorRanges;
    std::shared_ptr<std::atomic<bool>> stopFlag;
};

#endif //THREADING_HPP
