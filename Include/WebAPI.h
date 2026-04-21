#ifndef WEBAPI_H
#define WEBAPI_H

#include <App.h>
#include <opencv2/opencv.hpp>

struct PerClientData {};
class WebAPI : public uWS::App{
public:
  WebAPI(WebAPI &&) = delete;
  WebAPI(const WebAPI &) = delete;
  WebAPI &operator=(WebAPI &&) = delete;
  WebAPI &operator=(const WebAPI &) = delete;
  virtual ~WebAPI();
  void streamMain();

  static WebAPI &GetInstance();
private:
  WebAPI();
  std::mutex frame_mutex;
  cv::Mat current_frame;
  std::atomic<bool> running_stream;
  std::vector<uWS::WebSocket<false, true, PerClientData>*> clients;
};

#endif // !WEBAPI_H
