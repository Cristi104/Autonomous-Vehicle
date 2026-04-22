#include "../Include/Config.h"
#include <cmath>
#include <sstream>
#include <vector>

Config::Config() {
  values["on"] = 0;
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
  values["LD_angle_weights"] = std::vector<float>{0, 3, 2, 1, 0};
  values["LD_search_interval"] = 35;
  values["LD_search_range"] = 200;
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
std::string Config::json() {
  std::lock_guard<std::mutex> lock(mutex);
  std::stringstream stream;
  stream << "{\n";

  auto it = values.begin();
  while (it != values.end()) {
    const auto& [key, value] = *it;

    stream << "\"" << key << "\": ";

    std::visit([&stream](const auto& v) {
      using T = std::decay_t<decltype(v)>;

      if constexpr (std::is_same_v<T, int>) {
        stream << v;
      }
      else if constexpr (std::is_same_v<T, float>) {
        stream << v;
      }
      else if constexpr (std::is_same_v<T, std::vector<float>>) {
        stream << "[";
        for (int i = 0; i < v.size(); i++) {
          stream << v[i];
          if (i != v.size() - 1){
            stream << ", ";
          }
        }
        stream << "]";
      }
    }, value);

    // Only add a comma if it's NOT the last element
    if (std::next(it) != values.end()) {
      stream << ",\n";
    } else {
      stream << "\n"; // Newline for the last element, no comma
    }
    ++it;
  }

  stream << "}\n";
  return stream.str();
}
