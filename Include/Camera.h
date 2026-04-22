#ifndef CAMERA_H
#define CAMERA_H

#include <opencv2/opencv.hpp>

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
  cv::Mat frame;
  cv::VideoCapture cap;
  
};
#endif // !CAMERA_H
