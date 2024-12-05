#include "gstreamer_capture.hpp"

#include <nlohmann/json.hpp>
#include <opencv2/highgui.hpp>

#include "ffmpeg_capture.hpp"


GstreamerCapture::~GstreamerCapture() {
    release();
}

int GstreamerCapture::openStream(const std::string &url, const std::string &timeout) {
    gst_init(nullptr, nullptr);

    const std::string pipeStr =
            "udpsrc address=233.3.4.180 port=4180 ! decodebin ! videoconvert ! video/x-raw,format=BGR ! appsink name=mysink sync=0";

    GError *error = nullptr;

    pipeline = gst_parse_launch(pipeStr.c_str(), &error);
    checkErr(error);

    if (!pipeline) {
        return -1;
    }
    sinkVideo = gst_bin_get_by_name(GST_BIN(pipeline), "mysink");

    if (!sinkVideo) {
        return -1;
    }


    bus = gst_element_get_bus(pipeline);
    if (!bus) {
        std::cerr << "Failed to get bus from pipeline." << std::endl;
        return -1;
    }

    gst_app_sink_set_max_buffers(GST_APP_SINK(sinkVideo), 5);
    gst_app_sink_set_drop(GST_APP_SINK(sinkVideo), TRUE);
    gst_element_set_state(pipeline, GST_STATE_PLAYING);


    return 0;
}

int GstreamerCapture::grabFrame() {
    GstMessage *msg = gst_bus_timed_pop_filtered(bus, GST_CLOCK_TIME_NONE,
                                                 static_cast<GstMessageType>(GST_MESSAGE_ERROR | GST_MESSAGE_EOS));
    if (msg == nullptr) {
        return -1;
    }
    const int res = processBusMessage(pipeline, msg);
    gst_message_unref(msg);
    if (res < 0) {
        return -1;
    }
    return 0;
}

int GstreamerCapture::retrieveFrame(bool keyframesOnly) {
    if (gst_app_sink_is_eos(GST_APP_SINK(sinkVideo))) {
        std::cout << "eos" << std::endl;
        return -1;
    }

    sample = gst_app_sink_pull_sample(GST_APP_SINK(sinkVideo));

    if (sample == nullptr) {
        std::cout << "no sample" << std::endl;
        return -1;
    }
    caps = gst_sample_get_caps(sample);

    if (caps == nullptr) {
        std::cout << "no caps" << std::endl;
        return -1;
    }

    structure = gst_caps_get_structure(caps, 0);

    if (!gst_structure_get_int(structure, "width", &width)) {
        std::cout << "failed to get width" << std::endl;
        return -1;
    }
    if (!gst_structure_get_int(structure, "height", &height)) {
        std::cout << "failed to get height" << std::endl;
        return -1;
    }

    buffer = gst_sample_get_buffer(sample);
    if (buffer == nullptr) {
        std::cout << "failed to get buffer" << std::endl;
        return -1;
    }

    return DECODE_OK;
}

int GstreamerCapture::getCVFrame(cv::Mat &frame) const {
    GstMapInfo map;
    if (!gst_buffer_map(buffer, &map, GST_MAP_READ)) {
        std::cerr << "Failed to map buffer for reading!" << std::endl;
        return -1;
    }
    map.size = frame.rows * frame.cols * 3;
    const cv::Mat mat(height, width, CV_8UC3, map.data);
    mat.copyTo(frame);
    gst_buffer_unmap(buffer, &map);
    gst_sample_unref(sample);
    return 0;
}

void GstreamerCapture::release() {
    if (pipeline) {
        gst_element_set_state(pipeline, GST_STATE_NULL);
        gst_object_unref(pipeline);
        pipeline = nullptr;
    }
    if (sinkVideo) {
        gst_object_unref(sinkVideo);
        sinkVideo = nullptr;
    }
    if (sample) {
        gst_sample_unref(sample);
        sample = nullptr;
    }
    if (bus) {
        gst_object_unref(bus);
        bus = nullptr;
    }
    /*   if (structure) {
           gst_structure_free(structure);
           structure = nullptr;
       }*/
    if (buffer) {
        gst_buffer_unref(buffer);
        buffer = nullptr;
    }
    if (caps) {
        gst_caps_unref(caps);
        caps = nullptr;
    }
}

int GstreamerCapture::processBusMessage(GstElement *pipeline, GstMessage *msg) {
    switch (GST_MESSAGE_TYPE(msg)) {
        case GST_MESSAGE_ERROR: {
            GError *error = nullptr;
            gchar *dbg = nullptr;
            gst_message_parse_error(msg, &error, &dbg);
            std::cout << "Error: " << error->message << std::endl;
            if (dbg) {
                std::cout << "Debug Info: " << dbg << std::endl;
                g_free(dbg);
            }
            g_clear_error(&error);
            return -1;
        }
        case GST_MESSAGE_EOS:
            std::cout << "End of Stream" << std::endl;
            return -1;

        case GST_MESSAGE_STATE_CHANGED:
            std::cout << "State Changed" << std::endl;
            if (GST_MESSAGE_SRC(msg) == GST_OBJECT(pipeline)) {
                GstState sOld, sNew, sPenging;
                gst_message_parse_state_changed(msg, &sOld, &sNew, &sPenging);
                std::cout << "Pipeline changed from " << gst_element_state_get_name(sOld) << " to " <<
                        gst_element_state_get_name(sNew) << std::endl;
            }
            break;

        case GST_MESSAGE_STEP_START:
            std::cout << "Step Start!" << std::endl;
            break;

        case GST_MESSAGE_STREAM_STATUS:
            std::cout << "Stream Status!" << std::endl;
            break;

        case GST_MESSAGE_ELEMENT:
            std::cout << "Message Element!" << std::endl;
            break;

        default:
            std::cout << "[INFO] Unhandled Message Type: "

                    << GST_MESSAGE_TYPE_NAME(msg) << std::endl;
            break;
    }
    return 0;
}
