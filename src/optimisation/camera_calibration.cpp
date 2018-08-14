#include "camera_calibration.hpp"


struct FocusOfExpansionError {
    cv::Mat prevFrame_;
    cv::Mat currFrame_;
    double focal_length_;
    double optical_center_x_;
    double optical_center_y_;

    FocusOfExpansionError(cv::Mat prevFrame, cv::Mat currFrame, double focal_length, double optical_center_x,
                          double optical_center_y) : focal_length_(
            focal_length), prevFrame_(prevFrame), currFrame_(currFrame), optical_center_x_(optical_center_x),
                                                     optical_center_y_(optical_center_y) {}

    bool operator()(const double *const k,
                    const double *const p,
                    double *residuals) const {
        cv::Mat undistorted1;
        cv::Mat undistorted2;
        double cameraArray[] = {focal_length_, 0, optical_center_x_, 0, focal_length_, optical_center_y_, 0, 0, 1};
        cv::Mat mtx(3, 3, CV_64FC1, cameraArray);
        double paramArray[] = {k[0], k[1], p[0], p[1], k[2], k[3], k[4], k[5]};
        cv::Mat dist(8, 1, CV_64FC1, paramArray);
        undistort(prevFrame_, undistorted1, mtx, dist);
        undistort(currFrame_, undistorted2, mtx, dist);
        OpticalFlow driveRecorder(undistorted1);
        std::vector<std::vector<cv::Point2f>> features;
        driveRecorder.CalcOpticalFlow(undistorted2, features);
        double calibration = double(driveRecorder.CalcFocusOfExpansion(undistorted2));
        residuals[0] = calibration;
        return true;
    }
};

void CameraCalibration(cv::Mat prevFrame, cv::Mat currFrame, double paramK[6], double paramP[2], double paramC[3]) {
    double focal_length = paramC[0];
    double optical_center_x = paramC[1];
    double optical_center_y = paramC[2];

    google::InitGoogleLogging("CameraCalibration");
    double k1 = paramK[0];
    double k2 = paramK[1];
    double p1 = paramP[0];
    double k4 = paramK[3];
    double k5 = paramK[4];
    double k6 = paramK[5];
    double p2 = paramP[1];
    double k3 = paramK[2];
    double k[] = {k1, k2, k3, k4, k5, k6};
    double p[] = {p1, p2};
    ceres::Problem problem;
    ceres::CostFunction *cost_function =
            new ceres::NumericDiffCostFunction<FocusOfExpansionError, ceres::CENTRAL, 1, 6, 2>(
                    new FocusOfExpansionError(prevFrame, currFrame, focal_length, optical_center_x, optical_center_y));
    problem.AddResidualBlock(cost_function, NULL, k, p);

    ceres::Solver::Options options;
    options.linear_solver_type = ceres::DENSE_SCHUR;
    options.minimizer_progress_to_stdout = true;
    ceres::Solver::Summary summary;
    ceres::Solve(options, &problem, &summary);
    std::cout << summary.FullReport() << "\n";
    std::cout << "k1 : " << k[0] << std::endl;
    std::cout << "k2 : " << k[1] << std::endl;
    std::cout << "p1 : " << p[0] << std::endl;
    std::cout << "p2 : " << p[1] << std::endl;
    std::cout << "k3 : " << k[2] << std::endl;
    std::cout << "k4 : " << k[3] << std::endl;
    std::cout << "k5 : " << k[4] << std::endl;
    std::cout << "k6 : " << k[5] << std::endl;

    for (int i = 0; i < 6; i++) {
        paramK[i] = k[i];
    }
    for (int i = 0; i < 2; i++) {
        paramP[i] = p[i];
    }
}

