//
// Created by gio on 12/1/24.
//

#ifndef ICAPTURE_HPP
#define ICAPTURE_HPP
#include <string>

#include "config.hpp"

class ICapture {
public:
    virtual ~ICapture() = default;

    virtual int openStream(const std::string &url, const Config & config, const std::string &timeout) = 0;

    [[nodiscard]] virtual int grabFrame()  = 0;

    virtual int retrieveFrame(bool keyframesOnly) = 0;

    virtual int getCVFrame(cv::Mat &frame) const = 0;

    virtual void release() = 0;
};



#endif //ICAPTURE_HPP
