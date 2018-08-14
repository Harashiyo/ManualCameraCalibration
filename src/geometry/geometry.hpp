//
// Created by Haraoka Shohei on 2018/08/09.
//

#ifndef PITCHANGLECORRECTION_EPIPOLAR_GEOMETRY_HPP
#define PITCHANGLECORRECTION_EPIPOLAR_GEOMETRY_HPP

#include <opencv2/opencv.hpp>


float SolveY(cv::Vec3f efficient, float x);

float SolveX(cv::Vec3f efficient, float y);

void DrawLines(std::vector<cv::Vec3f> &lines, cv::Mat &image, float translateX = 0.0, float translateY = 0.0);

void DrawEpipolarLines(std::vector<cv::Point2f> &points, cv::Mat &fundamentalMat, cv::Mat &image);

cv::Vec3f CalcLines(float x1, float y1, float x2, float y2);

void CalcFundamentalMat(std::vector<std::vector<cv::Point2f>> &points,std::vector<std::vector<cv::Point2f>> &maskedPoints,cv::Mat &fundamentalMat);

float CalcDistance(cv::Vec3f line, float pointX, float pointY);


#endif //PITCHANGLECORRECTION_EPIPOLAR_GEOMETRY_HPP
