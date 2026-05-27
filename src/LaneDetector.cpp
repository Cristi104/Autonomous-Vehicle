#include "../Include/LaneDetector.h"
#include "../Include/Config.h"
#include <cstdlib>
#include <opencv2/core.hpp>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>
# define MAX_SIZE 1
LaneDetector::LaneDetector() {
  roi = cv::Rect_(100, 0, 440, 400);
  // Perspective transform
  cv::Point2f pts1[4], pts2[4];
  srcPts = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_transform_source"));
  dstPts = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_transform_dest"));
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
  M = cv::getPerspectiveTransform(pts1, pts2);
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
  leftPoints.clear();
  rightPoints.clear();
  midPoints.clear();
  angles.clear();
  cv::Mat tempFrame;
  image.copyTo(tempFrame);
  cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2GRAY);
  tempFrame = 255 - tempFrame;

  cv::warpPerspective(tempFrame, sourceFrame, M, cv::Size(
        std::get<int>(Config::GetInstance().getItem("LD_source_width")), 
        std::get<int>(Config::GetInstance().getItem("LD_source_height"))
  ));

  cv::adaptiveThreshold(sourceFrame, sourceFrame, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, -2);
  
  cv::line(sourceFrame,
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[0],
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[1]),
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[4], 
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[5]),
  0, 10);
  cv::line(sourceFrame,
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[2],
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[3]),
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[6], 
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[7]),
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
    cv::Point p1(l[0], l[1]);
    cv::Point p2(l[2], l[3]);
    cv::line(sourceFrame, p1, p2, cv::Scalar(255), 5);
  }

  cv::Mat sub = sourceFrame(roi);
  if(cv::countNonZero(sub) == 0) {
    Config::GetInstance().setItem("on", 0);
    if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
      return addAnotations(image);
    } else {
      return addAnotations(sourceFrame);
    }
  }

  cv::line(sourceFrame,
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[0],
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[1]),
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * (dstPts[4]-0.0), 
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[5]),
  255, 10);
  cv::line(sourceFrame,
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * dstPts[2],
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[3]),
    cv::Point(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * (dstPts[6]+0.0), 
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * dstPts[7]),
  255, 10);



  leftPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.20,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.99
  );
  leftPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.20,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.98
  );

  rightPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.70,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.99
  );
  rightPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.70,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.98
  );

  midPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.45,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.99
  );
  midPoints.emplace_back(
      std::get<int>(Config::GetInstance().getItem("LD_source_width")) * 0.45,
      std::get<int>(Config::GetInstance().getItem("LD_source_height")) * 0.98
  );

  angles.emplace_back(std::atan2(midPoints[0].y - midPoints[1].y, midPoints[0].x - midPoints[1].x) - CV_PI);

  int tries = 0;

  for (int i = 0; i < std::get<int>(Config::GetInstance().getItem("LD_search_points")); i++) {
    tries++;
    cv::Point white = getFirstWhitePoint(
        midPoints.back().x,
        midPoints.back().y,
        angles.back(),
        std::get<int>(Config::GetInstance().getItem("LD_search_interval"))
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
    if (lp.x != -1 || rp.x != -1) {
        double dx = midPoints[midPoints.size()-2].x - midPoints.back().x;
        double dy = midPoints[midPoints.size()-2].y - midPoints.back().y;

        angles.emplace_back(std::atan2(dy, dx) - CV_PI);
    }

  }
  
  double avg_angle = 0;
  double weight = 0;

  std::vector<float> weights = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_angle_weights"));
  for (size_t i = 0; i < angles.size() && i < weights.size(); i++) {
    avg_angle += -(angles[i] + CV_PI / 2) * weights[i];
    weight += weights[i];
  }

  if (weight > 0)
    avg_angle /= weight;

  if (pidQueue.empty() && avg_angle != 0) {
    pidQueue.push_back(avg_angle);
  }
  
  if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
    return addAnotations(image);
  } else {
    return addAnotations(sourceFrame);
  }
}

cv::Mat LaneDetector::addAnotations(const cv::Mat& image){
  cv::Mat tempFrame = image;
  if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
    cv::warpPerspective(image, tempFrame, M, cv::Size(
          std::get<int>(Config::GetInstance().getItem("LD_source_width")), 
          std::get<int>(Config::GetInstance().getItem("LD_source_height"))
    ));
    cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2GRAY);
  }
  cv::cvtColor(tempFrame, tempFrame, cv::COLOR_GRAY2BGR);

  for (auto& p : leftPoints)
    cv::circle(tempFrame, p, 5, cv::Scalar(255, 0, 0), -1);

  for (auto& p : rightPoints)
    cv::circle(tempFrame, p, 5, cv::Scalar(0, 255, 0), -1);

  for (size_t i = 1; i < midPoints.size(); i++)
    cv::line(tempFrame, midPoints[i-1], midPoints[i],
    cv::Scalar(0, 0, 255), 1);

  cv::rectangle(tempFrame, roi, cv::Scalar(127, 127, 0), 2);

  return tempFrame;

}

float LaneDetector::getPIDValue(){
  if (pidQueue.size() >= MAX_SIZE) {
    float value = pidQueue.front();
    pidQueue.pop_front();
    return value;
  }
  return 0;
}
