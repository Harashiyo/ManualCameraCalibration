#include "feature_detection.hpp"


void DetectCorners(cv::Mat &grayImage, std::vector<cv::Point2f> &output) {
    const int kMaxCorners = 50;
    const double kQualityLevel = 0.05;
    const double kMinDistance = 25;
    const int kBlockSize = 3;
    const bool kUseHarrisDetector = false;
    const double kK = 0.04;
    // Parameters:
    //      image               Input 8-bit or floating-point 32-bit, single-channel image.
    //      eig_image           The parameter is ignored.
    //      temp_image          The parameter is ignored.
    //      corners             Output vector of detected corners.
    //      maxCorners          Maximum number of corners to return. If there are more corners than are found, the strongest of them is returned.
    //      qualityLevel        Parameter characterizing the minimal accepted quality of image corners. The parameter value is multiplied by the best corner quality measure, which is the minimal eigenvalue (see cornerMinEigenVal() ) or the Harris function response (see cornerHarris() ). The corners with the quality measure less than the product are rejected. For example, if the best corner has the quality measure = 1500, and the qualityLevel=0.01 , then all the corners with the quality measure less than 15 are rejected.
    //      minDistance         Minimum possible Euclidean distance between the returned corners.
    //      mask                Optional region of interest. If the image is not empty (it needs to have the type CV_8UC1 and the same size as image ), it specifies the region in which the corners are detected.
    //      blockSize           Size of an average block for computing a derivative covariation matrix over each pixel neighborhood. See cornerEigenValsAndVecs() .
    //      useHarrisDetector   Parameter indicating whether to use a Harris detector (see cornerHarris()) or cornerMinEigenVal().
    //      k                   Free parameter of the Harris detector.
    cv::goodFeaturesToTrack(grayImage,
                            output,
                            kMaxCorners,
                            kQualityLevel,
                            kMinDistance,
                            cv::Mat(),
                            kBlockSize,
                            kUseHarrisDetector,
                            kK);

    const cv::Size kWinSize = cv::Size(21, 21);
    const cv::Size kZeroZone = cv::Size(-1, -1);
    const cv::TermCriteria kCriteria(cv::TermCriteria::COUNT | cv::TermCriteria::EPS, 30, 0.01);
    // Parameters:
    //      image	    Input image.
    //      corners	    Initial coordinates of the input corners and refined coordinates provided for output.
    //      winSize	    Half of the side length of the search window. For example, if winSize=Size(5,5) , then a 5∗2+1×5∗2+1=11×11 search window is used.
    //      zeroZone	Half of the size of the dead region in the middle of the search zone over which the summation in the formula below is not done. It is used sometimes to avoid possible singularities of the autocorrelation matrix. The value of (-1,-1) indicates that there is no such a size.
    //      criteria	Criteria for termination of the iterative process of corner refinement. That is, the process of corner position refinement stops either after criteria.maxCount iterations or when the corner position moves by less than criteria.epsilon on some iteration.
    cv::cornerSubPix(grayImage, output, kWinSize, kZeroZone, kCriteria);
}

void DetectFeatures(cv::Mat &image, std::vector<cv::Point2f> &output) {
    cv::Mat grayImage;
    if (image.channels() == 1) {
        grayImage = image.clone();
    } else {
        cv::cvtColor(image, grayImage, cv::COLOR_BGR2GRAY);
    }

    std::vector<cv::Point2f> result;

    const int kSplitNumber = 3;
    const int kSplitHeight = grayImage.rows / kSplitNumber;
    const int margin = 35;
    const int width = grayImage.cols - 1;
    const int height = grayImage.rows - 1;

    for (int i = 0; i < kSplitNumber; i++) {
        cv::Rect roi(0, kSplitHeight * i, grayImage.cols, kSplitHeight);
        cv::Mat splitGrayImage = grayImage(roi);
        std::vector<cv::Point2f> corners;
        DetectCorners(splitGrayImage, corners);
        if (i != 0) {
            for (int j = 0; j < corners.size(); j++) {
                corners[j].y += kSplitHeight * i;
            }
        }
        for (int j = 0; j < corners.size(); j++) {
            if (corners[j].x < margin || corners[j].x > width - margin || corners[j].y < margin ||
                corners[j].y > height - margin) {
                continue;
            }
            result.push_back(corners[j]);
        }
    }
    output = result;
}
