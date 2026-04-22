#include "../Include/Camera.h"
#include "../Include/Config.h"
#include <opencv2/imgproc.hpp>

Camera::Camera() {
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  std::string pipeline =
      "libcamerasrc ! video/x-raw,width=640,height=480,framerate=30/1 ! "
      "videoconvert ! appsink";
  
  cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);
#else 
  cap = cv::VideoCapture(0);
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
#else 
  cap >> frame;
  cv::resize(frame, frame, cv::Size(std::get<int>(Config::GetInstance().getItem("source_width")), std::get<int>(Config::GetInstance().getItem("source_height"))));
#endif
  return frame;
}

Camera &Camera::GetInstance() {
  static Camera cam;
  return cam;
}
