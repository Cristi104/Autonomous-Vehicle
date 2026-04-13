#include "../Include/App.h"
#include <opencv2/opencv.hpp>
App::App() {
}

App::~App() {
}
#include <iostream>

int main() {
    std::string pipeline =
        "libcamerasrc ! video/x-raw,width=640,height=480,framerate=30/1 ! "
        "videoconvert ! appsink";

    cv::VideoCapture cap(pipeline, cv::CAP_GSTREAMER);

    if (!cap.isOpened()) {
        std::cerr << "Failed to open camera!" << std::endl;
        return -1;
    }

    cv::Mat frame;
    while (true) {
        cap >> frame;
        if (frame.empty()) break;

        cv::imshow("Camera", frame);
        if (cv::waitKey(1) == 27) break;
    }

    return 0;
}
