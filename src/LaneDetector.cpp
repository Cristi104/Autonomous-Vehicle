#include "../Include/LaneDetector.h"
#include "../Include/Config.h"
#include <algorithm>
#include <cstdlib>
#include <ios>
#include <iostream>
#include <iterator>
#include <opencv2/core.hpp>
#include <opencv2/core/cvdef.h>
#include <opencv2/core/hal/interface.h>
#include <opencv2/core/mat.hpp>
#include <opencv2/core/types.hpp>
#include <opencv2/imgproc.hpp>
#include <ostream>
#include <utility>
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
  // cv::HoughLinesP(sourceFrame, linesP,
  //   1,
  //   std::get<float>(Config::GetInstance().getItem("LD_hough_theta")),
  //   std::get<int>(Config::GetInstance().getItem("LD_hough_thresh")),
  //   std::get<int>(Config::GetInstance().getItem("LD_hough_min_line_length")),
  //   std::get<int>(Config::GetInstance().getItem("LD_hough_max_line_gap"))
  // );
  // for (auto& l : linesP) {
  //   cv::Point p1(l[0], l[1]);
  //   cv::Point p2(l[2], l[3]);
  //   cv::line(sourceFrame, p1, p2, cv::Scalar(255), 1);
  // }
  // cv::morphologyEx(sourceFrame, sourceFrame, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
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
  // std::cout<<avg_angle<<std::endl;

  if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
    return addAnotations(image);
  } else {
    return addAnotations(sourceFrame);
  }
  // int LD_search_points = std::get<int>(Config::GetInstance().getItem("LD_search_points"));
  // int LD_search_interval = std::get<int>(Config::GetInstance().getItem("LD_search_interval"));
  // int LD_search_range = std::get<int>(Config::GetInstance().getItem("LD_search_range"));
  // leftPoints.clear();
  // rightPoints.clear();
  // midPoints.clear();
  // angles.clear();
  // pointsHough.clear();
  // cv::Mat tempFrame;
  // image.copyTo(tempFrame);
  // // cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2GRAY);
  // int LD_source_width = std::get<int>(Config::GetInstance().getItem("LD_source_width"));
  // int LD_source_height = std::get<int>(Config::GetInstance().getItem("LD_source_height"));
  //
  // cv::warpPerspective(tempFrame, sourceFrame, M, cv::Size(
  //   LD_source_width, 
  //   LD_source_height
  // ));
  //
  // // cv::adaptiveThreshold(sourceFrame, sourceFrame, 255, cv::ADAPTIVE_THRESH_MEAN_C, cv::THRESH_BINARY, 11, -2);
  //
  // // cv::erode(sourceFrame, sourceFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3)));
  // // cv::morphologyEx(sourceFrame, sourceFrame, cv::MORPH_OPEN, cv::getStructuringElement(cv::MORPH_CROSS, cv::Size(5, 5)));
  // // cv::morphologyEx(sourceFrame, sourceFrame, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5)));
  // cv::GaussianBlur(sourceFrame, sourceFrame, cv::Size(9, 9), 0);
  //
  // cv::Canny(sourceFrame, sourceFrame,
  //   std::get<int>(Config::GetInstance().getItem("LD_canny_min")),
  //   std::get<int>(Config::GetInstance().getItem("LD_canny_max"))
  // );
  // // if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
  // //   return addAnotations(image);
  // // } else {
  // //   return addAnotations(sourceFrame);
  // // }
  //
  // // cv::line(sourceFrame,
  // //   cv::Point(
  // //     LD_source_width * dstPts[0],
  // //     LD_source_height * dstPts[1]),
  // //   cv::Point(
  // //     LD_source_width * dstPts[4],
  // //     LD_source_height * dstPts[5]),
  // // 0, 5);
  // // cv::line(sourceFrame,
  // //   cv::Point(
  // //     LD_source_width * dstPts[2],
  // //     LD_source_height * dstPts[3]),
  // //   cv::Point(
  // //     LD_source_width * dstPts[6],
  // //     LD_source_height * dstPts[7]),
  // // 0, 5);
  //
  // // cv::cvtColor(sourceFrame, sourceFrame, cv::COLOR_GRAY2BGR);
  //
  //
  // // std::vector<cv::Vec4i> linesP;
  // // cv::HoughLinesP(sourceFrame, linesP,
  // //   1,
  // //   std::get<float>(Config::GetInstance().getItem("LD_hough_theta")),
  // //   std::get<int>(Config::GetInstance().getItem("LD_hough_thresh")),
  // //   std::get<int>(Config::GetInstance().getItem("LD_hough_min_line_length")),
  // //   std::get<int>(Config::GetInstance().getItem("LD_hough_max_line_gap"))
  // // );
  // // cv::line(sourceFrame,
  // //   cv::Point(
  // //     LD_source_width * 0,
  // //     LD_source_height * 1),
  // //   cv::Point(
  // //     LD_source_width * 0.45,
  // //     LD_source_height * 1),
  // // 255, 3);
  // // cv::line(sourceFrame,
  // //   cv::Point(
  // //     LD_source_width * 0.55,
  // //     LD_source_height * 1),
  // //   cv::Point(
  // //     LD_source_width * 1,
  // //     LD_source_height * 1),
  // // 255, 3);
  //
  // // cv::dilate(sourceFrame, sourceFrame, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(30, 30)));
  //
  //
  // // sourceFrame = cv::Mat::zeros(sourceFrame.size(), CV_8UC1);
  // // houghLinesToPoints(linesP, pointsHough, 40.0f);
  // // splitPointsByConnectedComponents(pointsHough, LD_source_width, LD_source_height, leftPoints, rightPoints, 60);
  // // int pointsPerSample = leftPoints.size() / LD_search_points;
  // // std::vector<cv::Point> leftPoints2(LD_search_points);
  // // if (pointsPerSample != 0) {
  // //   for(int i = 0; i < leftPoints.size(); i += pointsPerSample){
  // //     cv::Point p(0, 0);
  // //     int j = 0;
  // //     for (j = 0; j < pointsPerSample && i + j < leftPoints.size(); j++) {
  // //       p.x += leftPoints[i + j].x;
  // //       p.y += leftPoints[i + j].y;
  // //     }
  // //     if (j != 0) {
  // //       p.x /= j;
  // //       p.y /= j;
  // //       leftPoints2.push_back(p);
  // //     }
  // //   }
  // // }
  // // leftPoints = leftPoints2;
  // // pointsPerSample = rightPoints.size() / LD_search_points;
  // // std::vector<cv::Point> rightPoints2(LD_search_points);
  // // if (pointsPerSample != 0) {
  // //   for(int i = 0; i < rightPoints.size(); i += pointsPerSample){
  // //     cv::Point p(0, 0);
  // //     int j = 0;
  // //     for (j = 0; j < pointsPerSample && i + j < rightPoints.size(); j++) {
  // //       p.x += rightPoints[i + j].x;
  // //       p.y += rightPoints[i + j].y;
  // //     }
  // //     if (j != 0) {
  // //       p.x /= j;
  // //       p.y /= j;
  // //       rightPoints2.push_back(p);
  // //     }
  // //   }
  // // }
  // // rightPoints = rightPoints2;
  // // splitLeftRightByImageCenter(pointsHough, LD_source_width, leftPoints, rightPoints);
  // // leftPoints = orderPointsNearestNeighbor(leftPoints);
  // // rightPoints = orderPointsNearestNeighbor(rightPoints);
  // // leftPoints = simplifyPath(leftPoints);
  // // rightPoints = simplifyPath(rightPoints);
  //
  //
  // // for (auto& l : linesP) {
  // //   cv::Point p1(l[0], l[1]);
  // //   cv::Point p2(l[2], l[3]);
  // //   cv::line(sourceFrame, p1, p2, cv::Scalar(255), 1);
  // // }
  // // cv::morphologyEx(sourceFrame, sourceFrame, cv::MORPH_CLOSE, cv::getStructuringElement(cv::MORPH_RECT, cv::Size(50, 70)));
  //
  // // cv::Mat sub = sourceFrame(roi);
  // // if(cv::countNonZero(sub) == 0) {
  // //   Config::GetInstance().setItem("on", 0);
  // //   if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
  // //     return addAnotations(image);
  // //   } else {
  // //     return addAnotations(sourceFrame);
  // //   }
  // // }
  //
  // leftPoints.emplace_back(
  //     LD_source_width * 0.23,
  //     LD_source_height * 0.99
  // );
  // leftPoints.emplace_back(
  //     LD_source_width * 0.23,
  //     LD_source_height * 0.98
  // );
  //
  // rightPoints.emplace_back(
  //     LD_source_width * 0.77,
  //     LD_source_height * 0.99
  // );
  // rightPoints.emplace_back(
  //     LD_source_width * 0.77,
  //     LD_source_height * 0.98
  // );
  //
  // midPoints.emplace_back(
  //     LD_source_width * 0.47,
  //     LD_source_height * 0.99
  // );
  // midPoints.emplace_back(
  //     LD_source_width * 0.47,
  //     LD_source_height * 0.98
  // );
  //
  // angles.emplace_back(std::atan2(midPoints[0].y - midPoints[1].y, midPoints[0].x - midPoints[1].x) - CV_PI);
  //
  // // cv::Point left, right, tempPointLeft, tempPointRight;
  // // int leftMult = 1, rightMult = 1;
  // // for (int i = 0; i < LD_search_points; i++) {
  // //   left = getPointAlongLine(leftPoints.back().x, leftPoints.back().y, angles.back(), LD_search_interval * leftMult);
  // //   right = getPointAlongLine(rightPoints.back().x, rightPoints.back().y, angles.back(), LD_search_interval * rightMult);
  // //
  // //   // tempPointLeft = getPointAlongLine(left.x, left.y, angles.back() - CV_PI / 2, 2);
  // //   // tempPointLeft = getFirstWhitePoint(tempPointLeft.x, tempPointLeft.y, angles.back() - CV_PI / 2, LD_search_range);
  // //   // tempPointRight = getPointAlongLine(left.x, left.y, angles.back() + CV_PI / 2, 2);
  // //   // tempPointRight = getFirstWhitePoint(tempPointRight.x, tempPointRight.y, angles.back() + CV_PI / 2, LD_search_range);
  // //   tempPointLeft = getFirstWhitePoint(left.x, left.y, angles.back() - CV_PI / 2, LD_search_range);
  // //   tempPointRight = getFirstWhitePoint(left.x, left.y, angles.back() + CV_PI / 2, LD_search_range);
  // //   if (tempPointLeft.x != -1 && tempPointRight.x != -1) {
  // //     if (distance(left, tempPointLeft) > distance(left, tempPointRight)) {
  // //       left = tempPointRight;
  // //     } else {
  // //       left = tempPointLeft;
  // //     }
  // //   } else {
  // //     if (tempPointRight.x != -1) {
  // //       left = tempPointRight;
  // //     } else {
  // //       left = tempPointLeft;
  // //     }
  // //   }
  // //   // tempPointLeft = getPointAlongLine(right.x, right.y, angles.back() - CV_PI / 2, 2);
  // //   // tempPointLeft = getFirstWhitePoint(tempPointLeft.x, tempPointLeft.y, angles.back() - CV_PI / 2, LD_search_range);
  // //   // tempPointRight = getPointAlongLine(right.x, right.y, angles.back() + CV_PI / 2, 2);
  // //   // tempPointRight = getFirstWhitePoint(tempPointRight.x, tempPointRight.y, angles.back() + CV_PI / 2, LD_search_range);
  // //   tempPointLeft = getFirstWhitePoint(right.x, right.y, angles.back() - CV_PI / 2, LD_search_range);
  // //   tempPointRight = getFirstWhitePoint(right.x, right.y, angles.back() + CV_PI / 2, LD_search_range);
  // //   if (tempPointLeft.x != -1 && tempPointRight.x != -1) {
  // //     if (distance(right, tempPointLeft) > distance(right, tempPointRight)) {
  // //       right = tempPointRight;
  // //     } else {
  // //       right = tempPointLeft;
  // //     }
  // //   } else {
  // //     if (tempPointLeft.x != -1) {
  // //       right = tempPointLeft;
  // //     } else {
  // //       right = tempPointRight;
  // //     }
  // //   }
  // //
  // //
  // //   if (right.x == -1) {
  // //     rightMult++;
  // //   } else {
  // //     rightMult = 1;
  // //     rightPoints.push_back(right);
  // //   }
  // //   if (left.x == -1) {
  // //     leftMult++;
  // //   } else {
  // //     rightMult = 1;
  // //     leftPoints.push_back(left);
  // //   }
  // //
  // //   // rightPoints.emplace_back(rightPoints.back().x, rightPoints.back().y - LD_search_interval);
  // //   // leftPoints.emplace_back(leftPoints.back().x, leftPoints.back().y - LD_search_interval);
  // //
  // //   midpoints.push_back(cv::point(
  // //       (leftpoints.back().x + rightpoints.back().x) / 2,
  // //       (leftpoints.back().y + rightpoints.back().y) / 2
  // //   ));
  // //   double dx = midPoints[midPoints.size()-2].x - midPoints.back().x;
  // //   double dy = midPoints[midPoints.size()-2].y - midPoints.back().y;
  // //
  // //   angles.emplace_back(std::atan2(dy, dx) - CV_PI);
  // // }
  // int tries = 0;
  //
  // for (int i = 0; i < std::get<int>(Config::GetInstance().getItem("LD_search_points")); i++) {
  //   tries++;
  //   cv::Point white = getFirstWhitePoint(
  //       midPoints.back().x,
  //       midPoints.back().y,
  //       angles.back(),
  //       std::get<int>(Config::GetInstance().getItem("LD_search_interval"))
  //   );
  //
  //   cv::Point left, right;
  //
  //   if (white.x == -1) {
  //     left = getPointAlongLine(midPoints.back().x, midPoints.back().y, angles.back(),
  //         std::get<int>(Config::GetInstance().getItem("LD_search_interval")));
  //     right = left;
  //   } else {
  //     left = getPointAlongLine(white.x, white.y, angles.back() + CV_PI, 5);
  //     right = left;
  //   }
  //
  //   cv::Point lp = getFirstWhitePoint(left.x, left.y, angles.back() - CV_PI/2,
  //       std::get<int>(Config::GetInstance().getItem("LD_search_range")));
  //
  //   cv::Point rp = getFirstWhitePoint(right.x, right.y, angles.back() + CV_PI/2,
  //       std::get<int>(Config::GetInstance().getItem("LD_search_range")));
  //
  //   if (lp.x != -1) leftPoints.push_back(lp);
  //   if (rp.x != -1) rightPoints.push_back(rp);
  //
  //   if (!leftPoints.empty() && !rightPoints.empty()) {
  //       midPoints.push_back(cv::Point(
  //           (leftPoints.back().x + rightPoints.back().x) / 2,
  //           (leftPoints.back().y + rightPoints.back().y) / 2
  //       ));
  //   }
  //   if (lp.x != -1 || rp.x != -1) {
  //       double dx = midPoints[midPoints.size()-2].x - midPoints.back().x;
  //       double dy = midPoints[midPoints.size()-2].y - midPoints.back().y;
  //
  //       angles.emplace_back(std::atan2(dy, dx) - CV_PI);
  //   }
  //
  // }
  //
  // double avg_angle = 0;
  // double weight = 0;
  //
  // std::vector<float> weights = std::get<std::vector<float>>(Config::GetInstance().getItem("LD_angle_weights"));
  // for (size_t i = 0; i < angles.size() && i < weights.size(); i++) {
  //   avg_angle += -(angles[i] + CV_PI / 2) * weights[i];
  //   weight += weights[i];
  // }
  //
  // if (weight > 0)
  //   avg_angle /= weight;
  //
  // if (pidQueue.empty() && avg_angle != 0) {
  //   pidQueue.push_back(avg_angle);
  // }
  //
  // if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
  //   return addAnotations(image);
  // } else {
  //   return addAnotations(sourceFrame);
  // }
}

cv::Mat LaneDetector::addAnotations(const cv::Mat& image){
  cv::Mat tempFrame = image;
  if (!std::get<int>(Config::GetInstance().getItem("debug_view"))){
    cv::warpPerspective(image, tempFrame, M, cv::Size(
          std::get<int>(Config::GetInstance().getItem("LD_source_width")), 
          std::get<int>(Config::GetInstance().getItem("LD_source_height"))
    ));
    // cv::cvtColor(tempFrame, tempFrame, cv::COLOR_BGR2GRAY);
  }
  cv::cvtColor(tempFrame, tempFrame, cv::COLOR_GRAY2BGR);

  // cv::polylines(sourceFrame, leftPoints, false, cv::Scalar(255, 0, 0), 2, cv::LINE_AA);
  // cv::polylines(sourceFrame, rightPoints, false, cv::Scalar(0, 255, 0), 2, cv::LINE_AA);
  for (auto& p : leftPoints)
    cv::circle(tempFrame, p, 1, cv::Scalar(255, 0, 0), -1);

  for (auto& p : rightPoints)
    cv::circle(tempFrame, p, 1, cv::Scalar(0, 255, 0), -1);

  for (size_t i = 1; i < midPoints.size(); i++)
    cv::line(tempFrame, midPoints[i-1], midPoints[i],
    cv::Scalar(0, 0, 255), 1);

  // cv::rectangle(tempFrame, roi, cv::Scalar(127, 127, 0), 2);

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
