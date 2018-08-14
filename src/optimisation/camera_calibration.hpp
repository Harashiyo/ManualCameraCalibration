#ifndef CAMERACALIBRATION_EOFCOSTFUNCTION_HPP
#define CAMERACALIBRATION_EOFCOSTFUNCTION_HPP

#include "../optical_flow/optical_flow.hpp"
#include "../geometry/geometry.hpp"
#include <ceres/ceres.h>
#include <glog/logging.h>
#include <opencv2/opencv.hpp>

struct FocusOfExpansionError;
void CameraCalibration(cv::Mat prevFrame, cv::Mat currFrame,double paramK[6], double paramP[2], double paramC[3]);

#endif //CAMERACALIBRATION_EOFCOSTFUNCTION_HPP
