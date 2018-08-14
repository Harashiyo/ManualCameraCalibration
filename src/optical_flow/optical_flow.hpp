#ifndef PITCHANGLECORRECTION_OPTICAL_FLOW_H
#define PITCHANGLECORRECTION_OPTICAL_FLOW_H

#include "feature_detection.hpp"
#include "../geometry/geometry.hpp"
#include <opencv2/opencv.hpp>

class OpticalFlow {
private:
    cv::Mat prevFrameGray_;
    std::vector<cv::Point2f> prevFeatures_;
    std::vector< std::vector<cv::Point2f>> result_;
    std::vector< std::vector<cv::Point2f>> normalized_;

public:
    enum LineType {
        STRAIGHT_LINE,
        LINE_SEGMENT
    };
    OpticalFlow(cv::Mat &image);
    void CalcOpticalFlow(cv::Mat &image,std::vector< std::vector<cv::Point2f>> &output);
    void DrawOpticalFlow(cv::Mat &image, LineType l, cv::Mat &output);
    void PrintFeatures(cv::Mat &image,cv::Mat &output);
    void Normarization(std::vector< std::vector<cv::Point2f>> &output);
    float CalcFocusOfExpansion(cv::Mat &image);
};


#endif //PITCHANGLECORRECTION_OPTICAL_FLOW_H
