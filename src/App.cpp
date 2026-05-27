#include "../Include/App.h"
#include "../Include/WebAPI.h"
#include "../Include/Config.h"
#include "../Include/Camera.h"
#include <chrono>
#include <csignal>
#include <sstream>
#include <thread>
App::App() {
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
  this->contr.setSpeed(std::get<int>(Config::GetInstance().getItem("speed")));
  this->contr.startThread();
  std::signal(SIGINT, [](int signal){
    if (signal == SIGINT) {
      App::GetInstance().contr.stopThread();
      exit(0);
    }
  });
#endif
}

App::~App() {
}

App &App::GetInstance() {
  static App app;
  return app;
}

void App::run() {
  mainThread = std::thread(&App::main, this);
  WebAPI::GetInstance().run();
}

void App::main() {
  int running = 0;
  while (true) {
    auto time = std::chrono::high_resolution_clock::now();
    float val = ld.getPIDValue();
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
    contr.setPID(
        std::get<float>(Config::GetInstance().getItem("kp")),
        std::get<float>(Config::GetInstance().getItem("ki")),
        std::get<float>(Config::GetInstance().getItem("kd")),
        std::get<float>(Config::GetInstance().getItem("kdef"))
    );
    this->contr.setSpeed(std::get<int>(Config::GetInstance().getItem("speed")));
    if (std::get<int>(Config::GetInstance().getItem("on"))) {
      running = 1;
      if (val != 0) {
        contr.pid(val);
      }
    } else {
      if (running == 1) {
        running = 0;
        contr.forwardCm(1);
      }
    }
#endif
    frame = Camera::GetInstance().getFrame();
    if (frame.empty()) {
      continue;
    } else {
      frame = ld.processImage(frame);
    }
    std::ostringstream ss;
    ss << val;
    std::string text(ss.str());
    cv::putText(frame, text, cv::Point(0, 400), cv::FONT_HERSHEY_COMPLEX, 0.5, cv::Scalar(255, 0, 0));
    WebAPI::GetInstance().setFrame(frame);
    auto time2 = std::chrono::high_resolution_clock::now();
    std::cout << "[PROCES TIME] " << time2 - time << '\n';
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
  }
}
