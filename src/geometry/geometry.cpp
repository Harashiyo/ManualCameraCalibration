//
// Created by Haraoka Shohei on 2018/08/09.
//

#include "geometry.hpp"

float SolveY(cv::Vec3f efficient, float x) {
    return -(efficient[0] * x + efficient[2]) / efficient[1];
}

float SolveX(cv::Vec3f efficient, float y) {
    return -(efficient[1] * y + efficient[2]) / efficient[0];
}

void DrawLines(std::vector<cv::Vec3f> &lines, cv::Mat &image, float translateX, float translateY) {
    for (cv::Vec3f l : lines) {
        if (l[1]) {
            // Y軸と交わる直線の場合
            float width = image.size().width;
            cv::Point2f left(0.0 , SolveY(l, 0.0-translateX) + translateY);
            cv::Point2f right(width , SolveY(l, width-translateX) + translateY);
            cv::line(image, left, right, cv::Scalar(0, 0, 255), 2);
        } else {
            // Y軸に平行な直線の場合
            float x = -l[2] / l[0];
            cv::Point2f top(x + translateX, 0.0);
            cv::Point2f bottom(x + translateX, image.size().height);
            cv::line(image, top, bottom, cv::Scalar(0, 0, 255), 2);
        }
    }
}

cv::Vec3f CalcLines(float x1, float y1, float x2, float y2) {
    float a = y1 - y2;
    float b = x2 - x1;
    float c = x1 * y2 - x2 * y1;
    return cv::Vec3f(a, b, c);
}

void DrawEpipolarLines(std::vector<cv::Point2f> &points, cv::Mat &fundamentalMat, cv::Mat &image) {
    float point[points.size()*2];
    for(int i = 0;i<points.size();i++){
        point[i*2] = points[i].x;
        point[i*2+1] = points[i].y;
    }
    cv::Mat pointsMat(1, points.size(), CV_32FC2,point);
    std::vector<cv::Vec3f> lines;
    cv::computeCorrespondEpilines(pointsMat, 1, fundamentalMat, lines);
    DrawLines(lines, image, (float)image.cols / 2, (float)image.rows / 2);
}

void
CalcFundamentalMat(std::vector<std::vector<cv::Point2f>> &points, std::vector<std::vector<cv::Point2f>> &maskedPoints,

                   cv::Mat &fundamentalMat) {
    std::vector<uchar> mask;
    // Parameters:
    //      points1     Array of N points from the first image. The point coordinates should be floating-point (single or double precision).
    //      points2     Array of the second image points of the same size and format as points1 .
    //      method      Method for computing a fundamental matrix.
    //                      CV_FM_7POINT for a 7-point algorithm.  N = 7
    //                      CV_FM_8POINT for an 8-point algorithm.  N \ge 8
    //                      CV_FM_RANSAC for the RANSAC algorithm.  N \ge 8
    //                      CV_FM_LMEDS for the LMedS algorithm.  N \ge 8
    //      param1      Parameter used for RANSAC. It is the maximum distance from a point to an epipolar line in pixels, beyond which the point is considered an outlier and is not used for computing the final fundamental matrix. It can be set to something like 1-3, depending on the accuracy of the point localization, image resolution, and the image noise.
    //      param2      Parameter used for the RANSAC or LMedS methods only. It specifies a desirable level of confidence (probability) that the estimated matrix is correct.
    //      mask        Output array of N elements, every element of which is set to 0 for outliers and to 1 for the other points. The array is computed only in the RANSAC and LMedS methods. For other methods, it is set to all 1’s.
    cv::Mat f = cv::findFundamentalMat(points[0], points[1], mask, cv::FM_RANSAC, 3, 0.99);
    fundamentalMat = f;
    std::vector<cv::Point2f> maskedPoint1;
    std::vector<cv::Point2f> maskedPoint2;
    for (int i = 0; i < mask.size(); i++) {
        if (mask[i]) {
            maskedPoint1.push_back(points[0][i]);
            maskedPoint2.push_back(points[1][i]);
        }
    }
    std::vector<std::vector<cv::Point2f>> masked;
    masked.push_back(maskedPoint1);
    masked.push_back(maskedPoint2);
    maskedPoints = masked;
}

float CalcDistance(cv::Vec3f line, float pointX, float pointY) {
    return std::abs(line[0] * pointX + line[1] * pointY + line[2]) / std::sqrt(line[0] * line[0] + line[1] * line[1]);
}

