#ifndef LANE_DETECTOR_H
#define LANE_DETECTOR_H

#include <deque>
#include <opencv2/core/types.hpp>
#include <opencv2/opencv.hpp>
#include <vector>

class LaneDetector{
public:
  LaneDetector();
  LaneDetector(LaneDetector &&) = default;
  LaneDetector(const LaneDetector &) = default;
  LaneDetector &operator=(LaneDetector &&) = default;
  LaneDetector &operator=(const LaneDetector &) = default;
  ~LaneDetector();

  cv::Point getPointAlongLine(int x, int y, double theta, int distance);
  cv::Point getFirstWhitePoint(int x, int y, double theta, int length);
  cv::Mat processImage(const cv::Mat& image);
  float getPIDValue();
private:
  cv::Mat addAnotations(const cv::Mat& image);
  cv::Mat sourceFrame;
  cv::Mat M;
  cv::Rect roi;
  std::vector<float> srcPts;
  std::vector<float> dstPts;
  std::deque<cv::Mat> history;
  std::deque<float> pidQueue;

  std::vector<cv::Point> leftPoints;
  std::vector<cv::Point> rightPoints;
  std::vector<cv::Point> midPoints;
  std::vector<double> angles;
};

#endif // !LANE_DETECTOR_H
