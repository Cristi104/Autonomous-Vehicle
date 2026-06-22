#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>
#include <queue>

class Camera {
public:
  Camera(Camera &&) = delete;
  Camera(const Camera &) = delete;
  Camera &operator=(Camera &&) = delete;
  Camera &operator=(const Camera &) = delete;
  ~Camera();

  const cv::Mat &getFrame();
  static Camera &GetInstance();
protected:
  Camera();
private:
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
#else 
  std::queue<std::string> data;
#endif
  cv::Mat cameraMatrix;
  cv::Mat distCoeffs;
  cv::Rect validROI;
  cv::Mat newCameraMatrix;
  cv::Mat map1, map2;
  cv::Mat frame;
  cv::VideoCapture cap;
  
};
#endif // !CAMERA_H
