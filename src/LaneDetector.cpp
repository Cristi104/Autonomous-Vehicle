#include "../Include/LaneDetector.h"
#include "../Include/Config.h"
#include <vector>

LaneDetector::LaneDetector() {
}

LaneDetector::~LaneDetector() {
}

cv::Point LaneDetector::getPointAlongLine(int x, int y, double theta, int distance) {
  return cv::Point(
      static_cast<int>(x + distance * std::cos(theta)),
      static_cast<int>(y + distance * std::sin(theta))
  );
}

cv::Point LaneDetector::getFirstWhitePoint(int x, int y, double theta, int length) {
 for (int i = 0; i < length; i++) {
   int nx = static_cast<int>(x + i * std::cos(theta));
   int ny = static_cast<int>(y + i * std::sin(theta));

   if (nx < 0 || nx >= std::get<int>(Config::GetInstance().getItem("LD_source_width")) ||
     ny < 0 || ny >= std::get<int>(Config::GetInstance().getItem("LD_source_height")))
     break;

   if (sourceFrame.at<uchar>(ny, nx))
     return cv::Point(nx, ny);
   }
   return cv::Point(-1, -1);
}

cv::Mat LaneDetector::processImage(const cv::Mat& image) {
  cv::Mat tempFrame;
  image.copyTo(tempFrame);
  cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2GRAY);
  tempFrame = 255 - tempFrame;

  // Perspective transform
  cv::Point2f pts1[4], pts2[4];

  std::vector<float> srcPts = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_transform_source"));
  std::vector<float> dstPts = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_transform_dest"));
  for (int i = 0; i < 4; i++) {
    pts1[i] = cv::Point2f(
      std::get<int>(Config::GetInstance().getItem("source_width")) * srcPts[i * 2],
      std::get<int>(Config::GetInstance().getItem("source_height")) * srcPts[i * 2 + 1]
    );

    pts2[i] = cv::Point2f(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[i * 2],
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[i * 2 + 1]
    );
  }
  cv::Mat M = cv::getPerspectiveTransform(pts1, pts2);
  cv::warpPerspective(tempFrame, sourceFrame, M, cv::Size(
        std::get<int>(Config::GetInstance().getItem("LD_source_width")), 
        std::get<int>(Config::GetInstance().getItem("LD_source_height"))
  ));
  cv::adaptiveThreshold(sourceFrame, sourceFrame, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, -2);

  cv::line(sourceFrame,
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.2,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.5),
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.8, 
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.9),
  0, 10);

  cv::erode(sourceFrame, sourceFrame, cv::Mat());
  cv::GaussianBlur(sourceFrame, sourceFrame, cv::Size(9, 9), 0);

  cv::Canny(sourceFrame, sourceFrame,
    std::get<int>(Config::GetInstance().getItem("LD_canny_min")),
    std::get<int>(Config::GetInstance().getItem("LD_canny_max"))
  );

  std::vector<cv::Vec4i> linesP;
  cv::HoughLinesP(sourceFrame, linesP,
    1,
    std::get<float>(Config::GetInstance().getItem("LD_hough_theta")),
    std::get<int>(Config::GetInstance().getItem("LD_hough_thresh")),
    std::get<int>(Config::GetInstance().getItem("LD_hough_min_line_length")),
    std::get<int>(Config::GetInstance().getItem("LD_hough_max_line_gap"))
  );

  sourceFrame = cv::Mat::zeros(sourceFrame.size(), CV_8UC1);

  for (auto& l : linesP) {
    cv::line(sourceFrame,
      cv::Point(l[0], l[1]),
      cv::Point(l[2], l[3]),
      cv::Scalar(255), 3);
  }


  leftPoints.clear();
  rightPoints.clear();
  midPoints.clear();
  angles.clear();

  leftPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.30,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.99
  );
  leftPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.30,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.98
  );

  rightPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.70,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.99
  );
  rightPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.70,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.98
  );

  midPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.50,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.99
  );
  midPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.50,
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.98
  );

  angles.emplace_back(std::atan2(midPoints[0].y - midPoints[1].y, midPoints[0].x - midPoints[1].x) - CV_PI);

  int tries = 0;

  for (int i = 0; i < std::get<int>(Config::GetInstance().getItem("LD_search_points")); i++) {
    tries++;
    cv::Point white = getFirstWhitePoint(
        midPoints.back().x,
        midPoints.back().y,
        angles.back(),
        std::get<int>(Config::GetInstance().getItem("LD_search_points"))
    );

    cv::Point left, right;

    if (white.x == -1) {
      left = getPointAlongLine(midPoints.back().x, midPoints.back().y, angles.back(),
          std::get<int>(Config::GetInstance().getItem("LD_search_interval")));
      right = left;
    } else {
      left = getPointAlongLine(white.x, white.y, angles.back() + CV_PI, 5);
      right = left;
    }

    cv::Point lp = getFirstWhitePoint(left.x, left.y, angles.back() - CV_PI/2,
        std::get<int>(Config::GetInstance().getItem("LD_search_range")));

    cv::Point rp = getFirstWhitePoint(right.x, right.y, angles.back() + CV_PI/2,
        std::get<int>(Config::GetInstance().getItem("LD_search_range")));

    if (lp.x != -1) leftPoints.push_back(lp);
    if (rp.x != -1) rightPoints.push_back(rp);

    if (!leftPoints.empty() && !rightPoints.empty()) {
        midPoints.push_back(cv::Point(
            (leftPoints.back().x + rightPoints.back().x) / 2,
            (leftPoints.back().y + rightPoints.back().y) / 2
        ));
    }

    if (midPoints.size() >= 2) {
        double dx = midPoints[midPoints.size()-2].x - midPoints.back().x;
        double dy = midPoints[midPoints.size()-2].y - midPoints.back().y;

        angles.emplace_back(std::atan2(dy, dx) - CV_PI);
    }
  }
  
  double avg_angle = 0;
  double weight = 0;

  std::vector<float> weights = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_angle_weights"));
  for (size_t i = 0; i < angles.size() && i < 4; i++) {
    avg_angle += -(angles[i] + CV_PI / 2) * weights[i];
    weight += weights[i];
  }

  if (weight > 0)
    avg_angle /= weight;

  pidQueue.push_back(avg_angle);

  cv::cvtColor(sourceFrame, sourceFrame, cv::COLOR_GRAY2BGR);

  for (auto& p : leftPoints)
    cv::circle(sourceFrame, p, 5, cv::Scalar(255, 0, 0), -1);

  for (auto& p : rightPoints)
    cv::circle(sourceFrame, p, 5, cv::Scalar(0, 255, 0), -1);

  for (size_t i = 1; i < midPoints.size(); i++)
    cv::line(sourceFrame, midPoints[i-1], midPoints[i],
    cv::Scalar(0, 0, 255), 1);

  return sourceFrame;
}
float LaneDetector::getPIDValue(){
  if (pidQueue.size()>=1) {
    float value = pidQueue.front();
    pidQueue.pop_front();
    return value;
  }
  return 0;
}
