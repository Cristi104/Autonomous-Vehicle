#ifndef CONFIG_H
#define CONFIG_H

#include <algorithm>
#include <iostream>
#include <sstream>
#include <istream>
#include <map>
#include <mutex>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

class Config {
public:
  Config(Config &&) = delete;
  Config(const Config &) = delete;
  Config &operator=(Config &&) = delete;
  Config &operator=(const Config &) = delete;
  friend std::ostream &operator<<(std::ostream &stream, Config &config) {
    std::lock_guard<std::mutex> lock(config.mutex);
    for (const auto& [key, value] : config.values) {
      stream << key << ": ";

      std::visit([&stream](const auto& v) {
        using T = std::decay_t<decltype(v)>;

        if constexpr (std::is_same_v<T, int>) {
          stream << "int " << v;
        }
        else if constexpr (std::is_same_v<T, float>) {
          stream << "float " << v;
        }
        else if constexpr (std::is_same_v<T, std::vector<float>>) {
          stream << "vfloat ";
          for (const auto& f : v)
            stream << f << " ";
        }
      }, value);

      stream << "\n";
    }
    return stream;
  }
  friend std::istream &operator>>(std::istream &stream, Config &config) {
    std::lock_guard<std::mutex> lock(config.mutex);
    std::string line;

    // Read multiple lines until empty line
    while (std::getline(stream, line) && !line.empty()) {
      std::istringstream iss(line);

      std::string key, type;
      iss >> key >> type;
      key.erase(std::find(key.begin(), key.end(), ':'));

      if (type == "int") {
        int value;
        iss >> value;
        config.values[key] = value;
      }
      else if (type == "float") {
        float value;
        iss >> value;
        config.values[key] = value;
      }
      else if (type == "vfloat") {
        std::vector<float> vec;
        float item;
        while (iss >> item) {
          vec.push_back(item);
        }
        config.values[key] = vec;
      }
    }
    return stream;
  }


  static Config &GetInstance();

  std::variant<int, float, std::vector<float>> getItem(const std::string &name);
  template<typename T>
  void setItem(const std::string &name, T value){
    std::lock_guard<std::mutex> lock(mutex);
    values[name] = value;
  }
  std::string json();
protected:
  Config();
  ~Config();
private:
  std::map<std::string, std::variant<int, float, std::vector<float>>> values;
  std::mutex mutex;
};

#endif // !CONFIG_H

