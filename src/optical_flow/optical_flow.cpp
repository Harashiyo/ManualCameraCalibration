#include "optical_flow.hpp"
#include <iostream>


OpticalFlow::OpticalFlow(cv::Mat &image) {
    if (image.channels() == 1) {
        prevFrameGray_ = image.clone();
    } else {
        cv::cvtColor(image, prevFrameGray_, cv::COLOR_BGR2GRAY);
    }
    DetectFeatures(prevFrameGray_, prevFeatures_);
    result_ = std::vector<std::vector<cv::Point2f>>(2);
}

void OpticalFlow::CalcOpticalFlow(cv::Mat &image, std::vector<std::vector<cv::Point2f>> &output) {
    cv::Mat currFrameGray;
    if (image.channels() == 1) {
        currFrameGray = image.clone();
    } else {
        cv::cvtColor(image, currFrameGray, cv::COLOR_BGR2GRAY);
    }

    std::vector<cv::Point2f> nextPts;
    std::vector<uchar> featuresFound;
    std::vector<float> featuresErrors;
    const cv::Size kWinSize = cv::Size(21, 21);
    const int kMaxLevel = 3;
    const cv::TermCriteria kCriteria = cv::TermCriteria(cv::TermCriteria::COUNT + cv::TermCriteria::EPS, 30, 0.01);
    const int kFlags = 0;
    const double kMinEigThreshold = 1e-4;
    // Parameters:
    //      prevImg	            first 8-bit input image or pyramid constructed by buildOpticalFlowPyramid.
    //      nextImg	            second input image or pyramid of the same size and the same type as prevImg.
    //      prevPts	            vector of 2D points for which the flow needs to be found; point coordinates must be single-precision floating-point numbers.
    //      nextPts	            output vector of 2D points (with single-precision floating-point coordinates) containing the calculated new positions of input features in the second image; when OPTFLOW_USE_INITIAL_FLOW flag is passed, the vector must have the same size as in the input.
    //      status	            output status vector (of unsigned chars); each element of the vector is set to 1 if the flow for the corresponding features has been found, otherwise, it is set to 0.
    //      err	                output vector of errors; each element of the vector is set to an error for the corresponding feature, type of the error measure can be set in flags parameter; if the flow wasn't found then the error is not defined (use the status parameter to find such cases).
    //      winSize	            size of the search window at each pyramid level.
    //      maxLevel	        0-based maximal pyramid level number; if set to 0, pyramids are not used (single level), if set to 1, two levels are used, and so on; if pyramids are passed to input then algorithm will use as many levels as pyramids have but no more than maxLevel.
    //      criteria	        parameter, specifying the termination criteria of the iterative search algorithm (after the specified maximum number of iterations criteria.maxCount or when the search window moves by less than criteria.epsilon.
    //      flags	            operation flags:
    //                              OPTFLOW_USE_INITIAL_FLOW uses initial estimations, stored in nextPts; if the flag is not set, then prevPts is copied to nextPts and is considered the initial estimate.
    //                              OPTFLOW_LK_GET_MIN_EIGENVALS use minimum eigen values as an error measure (see minEigThreshold description); if the flag is not set, then L1 distance between patches around the original and a moved point, divided by number of pixels in a window, is used as a error measure.
    //      minEigThreshold     the algorithm calculates the minimum eigen value of a 2x2 normal matrix of optical flow equations (this matrix is called a spatial gradient matrix in [20]), divided by number of pixels in a window; if this value is less than minEigThreshold, then a corresponding feature is filtered out and its flow is not processed, so it allows to remove bad points and get a performance boost.
    cv::calcOpticalFlowPyrLK(
            prevFrameGray_,
            currFrameGray,
            prevFeatures_,
            nextPts,
            featuresFound,
            featuresErrors,
            kWinSize,
            kMaxLevel,
            kCriteria,
            kFlags,
            kMinEigThreshold);
    result_[0].clear();
    result_[1].clear();
    for (int i = 0; i < featuresFound.size(); i++) {
        if (!featuresFound[i]) {
            continue;
        }
        if (cv::norm(prevFeatures_[i] - nextPts[i]) > 35) {
            continue;
        }
        if (cv::norm(prevFeatures_[i] - nextPts[i]) < 1) {
            continue;
        }
        result_[0].push_back(prevFeatures_[i]);
        result_[1].push_back(nextPts[i]);
    }
    output = result_;

    prevFrameGray_ = currFrameGray.clone();
    prevFeatures_.clear();
    DetectFeatures(currFrameGray, prevFeatures_);
}

void OpticalFlow::DrawOpticalFlow(cv::Mat &image, LineType l, cv::Mat &output) {
    output = image.clone();
    switch (l) {
        case LINE_SEGMENT:
            for (int i = 0; i < result_[0].size(); i++) {
                cv::Point p1 = cv::Point((int) result_[0][i].x, (int) result_[0][i].y);
                cv::Point p2 = cv::Point((int) result_[1][i].x, (int) result_[1][i].y);
                line(output, p1, p2, cv::Scalar(0, 0, 255), 6);
            }
            break;
        case STRAIGHT_LINE:
            //直線:ax+by+c=0
            std::vector<cv::Vec3f> lines;
            for (int i = 0; i < result_[0].size(); i++) {
                lines.push_back(CalcLines(result_[0][i].x, result_[0][i].y, result_[1][i].x, result_[1][i].y));
            }
            DrawLines(lines, output);
            break;
    }
}

void OpticalFlow::PrintFeatures(cv::Mat &image, cv::Mat &output) {
    output = image.clone();
    const int kRadius = 4;
    const cv::Scalar kPrevFeaturesColor(255, 0, 255);
    const cv::Scalar kCurrFeaturesColor(0, 255, 255);
    for (int i = 0; i < result_[0].size(); i++) {
        cv::circle(output, result_[0][i], kRadius, kPrevFeaturesColor, -1, cv::LINE_8, 0);
        cv::circle(output, result_[1][i], kRadius, kCurrFeaturesColor, -1, cv::LINE_8, 0);
    }
}

void OpticalFlow::Normarization(std::vector<std::vector<cv::Point2f>> &output) {
    std::vector<std::vector<cv::Point2f>> result;
    result.insert(result.end(), result_.begin(), result_.end());
    const float kFocalLength = 1000;
    float x = (float) (prevFrameGray_.cols - 1) / 2;
    float y = (float) (prevFrameGray_.rows - 1) / 2;
    for (int i = 0; i < result[0].size(); i++) {
/*        result[0][i].x = (result[0][i].x - x) / kFocalLength;
        result[0][i].y = (result[0][i].y - y) / kFocalLength;
        result[1][i].x = (result[1][i].x - x) / kFocalLength;
        result[1][i].y = (result[1][i].y - y) / kFocalLength;*/
        result[0][i].x = result[0][i].x - x;
        result[0][i].y = result[0][i].y - y;
        result[1][i].x = result[1][i].x - x;
        result[1][i].y = result[1][i].y - y;
    }
    output = result;
}

void Filter(std::vector<cv::Vec3f> &lines, float x1, float y1, float x2, float y2) {
    std::vector<cv::Vec3f> result;
    for (cv::Vec3f l :lines) {
        if (l[1]) {
            float left = SolveY(l, x1);
            if (left >= y1 && left < y2) {
                result.push_back(l);
                continue;
            }
            float right = SolveY(l, x2);
            if (right >= y1 && right < y2) {
                result.push_back(l);
                continue;
            }
        }
        if (l[0]) {
            float top = SolveX(l, y1);
            if (top >= x1 && top < x2) {
                result.push_back(l);
                continue;
            }
            float bottom = SolveX(l, y2);
            if (bottom >= x1 && bottom < x2) {
                result.push_back(l);
            }
        }
    }
    lines = result;
}

float OpticalFlow::CalcFocusOfExpansion(cv::Mat &image) {
    std::vector<cv::Vec3f> lines;
    for (int i = 0; i < result_[0].size(); i++) {
        lines.push_back(CalcLines(result_[0][i].x, result_[0][i].y, result_[1][i].x, result_[1][i].y));
    }
    float x = (float) image.cols / 5;
    float y = (float) image.rows / 3;
    float xx = x / 9;
    float yy = y / 9;
    float min[3] = {1000000, 0, 0};

    Filter(lines, x * 2, y, x * 3, y * 2);

    int num = lines.size();

    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            float sum = 0.0;
            for (cv::Vec3f l:lines) {
                //sum += std::sqrt(CalcDistance(l, x * 2 + xx * i, y + yy * j));
                sum += CalcDistance(l, x * 2 + xx * i, y + yy * j);
            }
            sum /= num;
            if (min[0] > sum) {
                min[0] = sum;
                min[1] = x * 2 + xx * i;
                min[2] = y + yy * j;
            }
        }
    }
    x = min[1];
    y = min[2];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            float sum = 0.0;
            for (cv::Vec3f l:lines) {
                //sum += std::sqrt(CalcDistance(l, x + i - 5, y + j - 5));
                sum += CalcDistance(l, x + i - 5, y + j - 5);
            }
            sum /= num;
            if (min[0] > sum) {
                min[0] = sum;
                min[1] = x + i - 5;
                min[2] = y + j - 5;
            }
        }
    }
    return min[0];
}
