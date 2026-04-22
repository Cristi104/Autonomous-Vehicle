#ifndef APP_H
#define APP_H

#include "../Include/LaneDetector.h"
#include "HWController/controller.h"
#include <opencv2/opencv.hpp>
#include <thread>

class App {
public:
  App(App &&) = delete;
  App(const App &) = delete;
  App &operator=(App &&) = delete;
  App &operator=(const App &) = delete;
  ~App();
  
  static App &GetInstance();
  void run();
  void main();
protected:
  App();
private:
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  Controller contr;
#endif
  std::thread mainThread;
  cv::Mat frame;
  LaneDetector ld;
};

#endif // !APP_H

