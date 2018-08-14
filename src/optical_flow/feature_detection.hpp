#ifndef PITCHANGLECORRECTION_FEATURES_DETECTION_H
#define PITCHANGLECORRECTION_FEATURES_DETECTION_H

#include <opencv2/opencv.hpp>

void DetectFeatures(cv::Mat &grayImage, std::vector<cv::Point2f> &output);


#endif //PITCHANGLECORRECTION_FEATURES_DETECTION_H
