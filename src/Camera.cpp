#include "../Include/Camera.h"
#include "../Include/Config.h"
#include "../Include/Config.h"
#include <opencv2/imgproc.hpp>
#include <format>

Camera::Camera() {
  int source_width = std::get<int>(Config::GetInstance().getItem("source_width"));
  int source_height = std::get<int>(Config::GetInstance().getItem("source_height"));
  cv::setUseOptimized(true);
  std::cout << "OpenCV optimized: " << cv::useOptimized() << std::endl;
  std::cout << "OpenCV threads: " << cv::getNumThreads() << std::endl;
  cameraMatrix = (cv::Mat_<double>(3,3) << 1479.55281, 0, 617.235232, 0, 1484.82551, 366.691817, 0, 0, 1);
  distCoeffs = (cv::Mat_<double>(1,5) << -0.45240936, 0.3584162, 0.00373871, -0.00591737, -0.52074064);
  cv::Size imageSize(source_width, source_height);
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  std::string pipeline =
      std::format("libcamerasrc ! video/x-raw,width={},height={},framerate=10/1 ! videoconvert ! appsink", source_width, source_height);
  
  cap = cv::VideoCapture(pipeline, cv::CAP_GSTREAMER);

#else 
  cap = cv::VideoCapture("./data4/video0.h264");
  for (int i = 1; i < 2; i++) {
    data.emplace(std::format("./data4/video{}.h264", i));
  }
#endif
  newCameraMatrix = cv::getOptimalNewCameraMatrix(
    cameraMatrix,
    distCoeffs,
    imageSize,
    0.0,
    imageSize,
    &validROI
  );

  cv::initUndistortRectifyMap(
    cameraMatrix,
    distCoeffs,
    cv::Mat(),
    newCameraMatrix,
    imageSize,
    CV_16SC2,
    map1,
    map2
  );

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
  if (frame.empty()) {
    cap.release();
    cap = cv::VideoCapture(data.front());
    data.pop();
    cap >> frame;
  }
  cv::resize(frame, frame, cv::Size(std::get<int>(Config::GetInstance().getItem("source_width")), std::get<int>(Config::GetInstance().getItem("source_height"))));
#endif
  cv::remap(
    frame,
    frame,
    map1,
    map2,
    cv::INTER_LINEAR,
    cv::BORDER_CONSTANT
  );

  cv::cvtColor(frame, frame, cv::COLOR_BGR2GRAY);
  return frame;
}

Camera &Camera::GetInstance() {
  static Camera cam;
  return cam;
}
