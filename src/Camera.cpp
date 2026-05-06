#include "../Include/Camera.h"
#include "../Include/Config.h"
#include "../Include/Config.h"
#include <opencv2/imgproc.hpp>
#include <format>


Camera::Camera() {
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  int source_width = std::get<int>(Config::GetInstance().getItem("source_width"));
  int source_height = std::get<int>(Config::GetInstance().getItem("source_height"));
  std::string pipeline =
      std::format("libcamerasrc ! video/x-raw,width={},height={},framerate=10/1 ! videoconvert ! appsink", source_width, source_height);
  
  cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);
#else 
  cap = cv::VideoCapture("./data/video0.mp4");
  for (int i = 1; i < 16; i++) {
    data.emplace(std::format("./data/video{}.mp4", i));
  }
#endif

  if (!cap.isOpened()) {
      std::cerr << "Failed to open camera!" << std::endl;
  }
}

Camera::~Camera() {
}

const cv::Mat &Camera::getFrame() {
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  cap >> frame;
  cv::rotate(frame, frame, cv::ROTATE_180);
#else 
  cap >> frame;
  if (frame.empty()) {
    cap.release();
    cap = cv::VideoCapture(data.front());
    data.pop();
    cap >> frame;
  }
  cv::resize(frame, frame, cv::Size(std::get<int>(Config::GetInstance().getItem("source_width")), std::get<int>(Config::GetInstance().getItem("source_height"))));
#endif
  return frame;
}

Camera &Camera::GetInstance() {
  static Camera cam;
  return cam;
}
