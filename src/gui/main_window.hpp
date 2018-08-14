//
// Created by Haraoka Shohei on 2018/08/13.
//

#ifndef CAMERACALIBRATION_CALIBRATION_APP_HPP
#define CAMERACALIBRATION_CALIBRATION_APP_HPP

#include "../image/image_io.hpp"
#include "../optimisation/camera_calibration.hpp"
#include <string>
#include <boost/format.hpp>
#include <math.h>
#include <QApplication>
#include <QWidget>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QFileDialog>
#include <opencv2/opencv.hpp>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QSlider>
#include <QtWidgets/QDialog>

class CalibrationApp : public QWidget {
Q_OBJECT

public:
    explicit CalibrationApp(QWidget *parent = 0);
private slots:
    void handleInputButton();
    void handleOutputButton();
    void handleConvertButton();
    void handleOptimiseButton();

private:
    QPushButton *inputButton;
    QPushButton *outputButton;
    QPushButton *convertButton;
    QPushButton *optimiseButton;
    QLabel *mainImage;
    QLabel *inputLabel;
    QLabel *outputLabel;
    float fl;
    int ocx;
    int ocy;
    float k1;
    float k2;
    float p1;
    float p2;
    float k3;
    float k4;
    float k5;
    float k6;
    QSpinBox *ocXSpin;
    QSpinBox *oc_y_spin;
    cv::Mat imageMat;
    std::string outputPath;
    std::vector<std::string> inputFiles;
    QLabel *k1_label;
    QLabel *k2_label;
    QLabel *k3_label;
    QLabel *k4_label;
    QLabel *k5_label;
    QLabel *k6_label;
    QLabel *p1_label;
    QLabel *p2_label;

private:
    QPixmap Mat2Pixmap(cv::Mat &image);
    QPixmap Undistord();
    void InitUI();
    float calcFocalLength(float max_value, float min_value, float value);
    float calcDistortionCoefficient(float max_value, float min_value, float value);
    void InitSpinBox();
};

#endif //CAMERACALIBRATION_CALIBRATION_APP_HPP
