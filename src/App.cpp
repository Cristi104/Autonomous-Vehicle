#include "../Include/App.h"
#include "../Include/WebAPI.h"
#include "../Include/Config.h"
#include "../Include/Camera.h"
App::App() {

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

  while (true) {
#if defined(__linux__) && (defined(__arm__) || defined(__aarch64__))
    contr.setPID(
        std::get<float>(Config::GetInstance().getItem("LD_kp")),
        std::get<float>(Config::GetInstance().getItem("LD_ki")),
        std::get<float>(Config::GetInstance().getItem("LD_kd")),
        std::get<float>(Config::GetInstance().getItem("LD_kdef"))
    );
    if (!std::get<int>(Config::GetInstance().getItem("on"))) {
      contr.pid(ld.getPIDValue());
    }
#endif
    frame = Camera::GetInstance().getFrame();
    if (frame.empty()) {
      continue;
    } else {
      frame = ld.processImage(frame);
    }
    WebAPI::GetInstance().setFrame(frame);
  }
}
