#include "../Include/WebAPI.h"
#include "../Include/Config.h"
#include "libusockets.h"
#include <App.h>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <sstream>
#include <string>
#include <thread>
#include <variant>
#include <vector>
#include <cctype>
#include <opencv2/opencv.hpp> // For OpenCV image processing

const std::string base64_chars =
             "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
             "abcdefghijklmnopqrstuvwxyz"
             "0123456789+/";

// Helper function to trim leading/trailing whitespace
std::string trim(const std::string& str) {
    auto wsfront = std::find_if_not(str.begin(), str.end(), [](int c){ return std::isspace(c); });
    auto wsback = std::find_if_not(str.rbegin(), str.rend(), [](int c){ return std::isspace(c); }).base();
    return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

// --- Base64 Encoding Function ---
// This is a common, simple Base64 implementation. For very high-performance
// or production-grade applications, consider a more optimized library.

std::string base64_encode(const std::vector<uchar>& in) {
    std::string out;
    int val = 0, valb = -6;
    for (uchar c : in) {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0) {
            out.push_back(base64_chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }
    if (valb > -6) {
        out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }
    while (out.size() % 4) {
        out.push_back('=');
    }
    return out;
}
// --- End Base64 Encoding ---
using ValueType = std::variant<int, float, std::vector<float>>;

void updateVariantIfTypeMatches(ValueType& var, const std::string& input_str) {
    bool updated = false;

    // 1. Try to parse as int
    // Check if the variant currently holds an int
    if (std::holds_alternative<int>(var)) {
        try {
            size_t pos;
            // Attempt to convert the string to an int
            int parsed_int = std::stoi(input_str, &pos);
            // Crucial check: Ensure the entire string was consumed by stoi.
            // This prevents "123abc" from being parsed as 123.
            if (pos == input_str.length()) {
                var = parsed_int; // Update the variant
                std::cout << "Updated variant to int: " << parsed_int << std::endl;
                updated = true;
            }
        } catch (const std::invalid_argument& e) {
            // String was not a valid int
        } catch (const std::out_of_range& e) {
            // String represented an int too large/small for 'int'
        }
    }
    if (updated) return; // If updated, we're done

    // 2. Try to parse as float
    // Check if the variant currently holds a float
    if (std::holds_alternative<float>(var)) {
        try {
            size_t pos;
            // Attempt to convert the string to a float
            float parsed_float = std::stof(input_str, &pos);
            // Ensure the entire string was consumed
            if (pos == input_str.length()) {
                var = parsed_float; // Update the variant
                std::cout << "Updated variant to float: " << parsed_float << std::endl;
                updated = true;
            }
        } catch (const std::invalid_argument& e) {
            // String was not a valid float
        } catch (const std::out_of_range& e) {
            // String represented a float too large/small for 'float'
        }
    }
    if (updated) return; // If updated, we're done
}

WebAPI::WebAPI() : uWS::App(), running_stream(true) {
  current_frame = cv::Mat(480, 640, CV_8UC3, cv::Scalar(255, 255, 0));
  this->get("/config", [](auto *res, auto *req){
    res->writeHeader("Content-Type", "application/json");
    res->writeHeader("Access-Control-Allow-Origin", "*");
    res->writeHeader("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
    res->writeHeader("Access-Control-Allow-Headers", "Content-Type, Authorization");
    res->writeHeader("Access-Control-Max-Age", "86400");
    res->end(Config::GetInstance().json());
  });
  this->ws<PerClientData>("/ws/control", {
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024,
    .idleTimeout = 0,
    .open = [](auto *ws) {
      std::cout << "Client connected" << std::endl;
    },
    .message = [](auto *ws, std::string_view message, uWS::OpCode opCode) {
      if (opCode == uWS::OpCode::TEXT) {
        std::cout << "Received: " << message << std::endl;
        std::stringstream iss;
        iss << message;
        // std::cout << iss.str() << std::endl;
        std::string key, value;
        iss >> key >> value;
        key.erase(std::find(std::begin(key), std::end(key), ':'));
        ValueType variant = Config::GetInstance().getItem(key);
        updateVariantIfTypeMatches(variant, value);
        Config::GetInstance().setItem(key, variant);
        std::cout << key << value << std::endl;
      }
    },
    .close = [](auto *ws, int code, std::string_view msg) {
      std::cout << "Client disconnected" << std::endl;
    }
  });
  this->ws<PerClientData>("/ws/video", {
    .compression = uWS::SHARED_COMPRESSOR,
    .maxPayloadLength = 16 * 1024,
    .idleTimeout = 0,
    .open = [this](auto *ws) {
      std::cout << "Client connected" << std::endl;
      clients.push_back(ws);
    },
    .close = [this](auto *ws, int code, std::string_view msg) {
      std::cout << "Client disconnected" << std::endl;
      clients.erase(std::remove(clients.begin(), clients.end(), ws), clients.end());
    }

  });
  this->listen(3000, [](auto *token) {
    if (token) {
        std::cout << "Listening on port 3000\n";
    } else {
        std::cout << "Failed to listen\n";
    }
  });

  us_timer_t *timer = us_create_timer((us_loop_t *)uWS::Loop::get(), 0, 0);

  us_timer_set(timer, [](us_timer_t *t) {
      std::vector<uchar> buf;
      std::vector<int> p;
      if (WebAPI::GetInstance().clients.size() == 0) {
        return ;
      }
      p.push_back(cv::IMWRITE_JPEG_QUALITY);
      p.push_back(90);

      WebAPI::GetInstance().frame_mutex.lock();
      cv::imencode(".jpg", WebAPI::GetInstance().current_frame, buf, p);

      std::string data = base64_encode(buf);

      for (auto *client : WebAPI::GetInstance().clients) {
        if (client->getBufferedAmount() < 1024 * 1024) {
          client->send(data, uWS::OpCode::TEXT);
        }
      }
      WebAPI::GetInstance().frame_mutex.unlock();
  }, 30, 30);
}

WebAPI &WebAPI::GetInstance(){
  static WebAPI api;
  return api;
}

WebAPI::~WebAPI() {
}
