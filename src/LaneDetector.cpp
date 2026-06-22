#include "../Include/LaneDetector.h"
#include "../Include/Config.h"
#include <cstdlib>
#include <opencv2/core.hpp>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <vector>

LaneDetector::LaneDetector() {
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
  angles.clear();
  leftPoints.clear();
  rightPoints.clear();
  int LD_source_width = std::get<int>(Config::GetInstance().getItem("LD_source_width"));
  int LD_source_height = std::get<int>(Config::GetInstance().getItem("LD_source_height"));
  int startX = LD_source_width/2;
  if (midPoints.size() >= 2) {
    startX = midPoints[1].x;
  }
  midPoints.clear();
  cv::Mat tempFrame;
  image.copyTo(tempFrame);
  midPoints.emplace_back(
      LD_source_width * 0.47,
      LD_source_height * 0.99
  );
  
  cv::warpPerspective(tempFrame, sourceFrame, M, cv::Size(
    LD_source_width, 
    LD_source_height
  ));
  cv::GaussianBlur(sourceFrame, sourceFrame, cv::Size(9, 9), 0);
  
  cv::Point left(-1, -1), right(-1, -1);
  cv::Canny(sourceFrame, sourceFrame,
    std::get<int>(Config::GetInstance().getItem("LD_canny_min")),
    std::get<int>(Config::GetInstance().getItem("LD_canny_max"))
  );
  cv::line(sourceFrame,
    cv::Point(
      LD_source_width * dstPts[0],
      LD_source_height * dstPts[1]),
    cv::Point(
      LD_source_width * dstPts[4],
      LD_source_height * dstPts[5]),
  255, 1);
  cv::line(sourceFrame,
    cv::Point(
      LD_source_width * dstPts[2],
      LD_source_height * dstPts[3]),
    cv::Point(
      LD_source_width * dstPts[6],
      LD_source_height * dstPts[7]),
  255, 1);
  std::vector<cv::Vec4i> linesP;
  for (int i = 0; i < LD_source_width; i++) {
    if (sourceFrame.at<uchar>(int(LD_source_height *0.9), i)) {
      if (i < startX) {
        left = cv::Point(i, int(LD_source_height *0.9));
      } else {
        right = cv::Point(i, int(LD_source_height *0.9));
        break;
      }
    }
  }
  leftPoints.push_back(left);
  rightPoints.push_back(right);
  midPoints.emplace_back(
      (leftPoints.back().x + rightPoints.back().x) / 2,
      (leftPoints.back().y + rightPoints.back().y) / 2
  );

  int range = 100;
  // left
  for (int i = 0; i < range; i++) {
    left = leftPoints.back();
    if (sourceFrame.at<uchar>(left.y-1, left.x+1)) {
      leftPoints.emplace_back(left.x+1, left.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(left.y-1, left.x)) {
      leftPoints.emplace_back(left.x, left.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(left.y-1, left.x-1)) {
      leftPoints.emplace_back(left.x-1, left.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(left.y, left.x+1)) {
      leftPoints.emplace_back(left.x+1, left.y);
      continue;
    }
    break;
  }
  // right
  for (int i = 0; i < range ; i++) {
    right = rightPoints.back();
    if (sourceFrame.at<uchar>(right.y-1, right.x+1)) {
      rightPoints.emplace_back(right.x+1, right.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(right.y-1, right.x)) {
      rightPoints.emplace_back(right.x, right.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(right.y-1, right.x-1)) {
      rightPoints.emplace_back(right.x-1, right.y-1);
      continue;
    }
    if (sourceFrame.at<uchar>(right.y, right.x-1)) {
      rightPoints.emplace_back(right.x-1, right.y);
      continue;
    }
    break;
  }
  for (int i = 0; i < leftPoints.size() && i < rightPoints.size(); i++){
    midPoints.emplace_back(
        (leftPoints[i].x + rightPoints[i].x) / 2,
        (leftPoints[i].y + rightPoints[i].y) / 2
    );
  }

  cv::Point lastMid(-1, -1);
  float sideMult = 1.2;
  if (midPoints.size() > (range/10) * 2 + 1) {
    for (int i = 0; i < midPoints.size(); i+= range/10){
      if (lastMid.x != -1) {
        angles.emplace_back(std::atan2(lastMid.y - midPoints[i].y, lastMid.x - midPoints[i].x) - CV_PI);
      }
      lastMid = midPoints[i];
    }
  } else {
    if (leftPoints.size() > rightPoints.size()) {
      for (int i = 0; i < leftPoints.size(); i+= range/10){
        if (lastMid.x != -1) {
          angles.emplace_back((std::atan2(lastMid.y - leftPoints[i].y, lastMid.x - leftPoints[i].x) - CV_PI) * sideMult);
        }
        lastMid = leftPoints[i];
      }
    } else {
      for (int i = 0; i < rightPoints.size(); i+= range/10){
        if (lastMid.x != -1) {
          angles.emplace_back((std::atan2(lastMid.y - rightPoints[i].y, lastMid.x - rightPoints[i].x) - CV_PI) * sideMult);
        }
        lastMid = rightPoints[i];
      }
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
  }
  cv::cvtColor(tempFrame, tempFrame, cv::COLOR_GRAY2BGR);

  for (auto& p : leftPoints)
    cv::circle(tempFrame, p, 1, cv::Scalar(255, 0, 0), -1);

  for (auto& p : rightPoints)
    cv::circle(tempFrame, p, 1, cv::Scalar(0, 255, 0), -1);

  for (size_t i = 1; i < midPoints.size(); i++)
    cv::line(tempFrame, midPoints[i-1], midPoints[i],
    cv::Scalar(0, 0, 255), 1);

  return tempFrame;

}

float LaneDetector::getPIDValue(){
  if (pidQueue.size() >= 1) {
    float value = pidQueue.front();
    pidQueue.pop_front();
    return value;
  }
  return 0;
}
