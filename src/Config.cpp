#include "../Include/Config.h"
#include <cmath>
#include <vector>

Config::Config() {
  values["source_width"] = 1296;
  values["source_height"] = 972;
  values["speed"] = 50;
  values["kp"] = 0.4f;
  values["ki"] = 0.0f;
  values["kd"] = 0.0f;
  values["kdef"] = 0.0f;
  values["LD_source_width"] = 640;
  values["LD_source_height"] = 480;
  values["LD_transform_source"] = std::vector<float>{0, 0, 1, 0, 0, 1, 1, 1};
  values["LD_transform_dest"] = std::vector<float>{0, 0, 1, 0, 0.3, 1, 0.7, 1};
  values["LD_search_interval"] = 35;
  values["LD_search_range"] = 460;
  values["LD_search_points"] = 8;
  values["LD_canny_min"] = 200;
  values["LD_canny_max"] = 250;
  values["LD_hough_theta"] = M_PIf/180.0f;
  values["LD_hough_thresh"] = 50;
  values["LD_hough_min_line_length"] = 10;
  values["LD_hough_max_line_gap"] = 50;
}

Config::~Config() {
}

Config &Config::GetInstance() {
  static Config instance;
  return instance;
}
std::variant<int, float, std::vector<float>> Config::getItem(const std::string &name) {
  std::lock_guard<std::mutex> lock(mutex);
  return values.at(name);
}
void Config::setItem(const std::string &name, int value) {
  std::lock_guard<std::mutex> lock(mutex);
  values[name] = value;
}
void Config::setItem(const std::string &name, float value) {
  std::lock_guard<std::mutex> lock(mutex);
  values[name] = value;
}
void Config::setItem(const std::string &name, std::vector<float> value) {
  std::lock_guard<std::mutex> lock(mutex);
  values[name] = value;
}
