#include <QtWidgets/QProgressBar>
#include "main_window.hpp"

CalibrationApp::CalibrationApp(QWidget *parent) : QWidget(parent) {
    InitUI();

    QStringList argList = QCoreApplication::arguments();
    if (argList.size() > 1) {
        SearchDir(argList[1].toStdString(), inputFiles);
        imageMat = cv::imread(inputFiles[0]);
        mainImage->setPixmap(Mat2Pixmap(imageMat));
        inputLabel->setText(argList[1]);
        ocx = imageMat.cols / 2;
        ocy = imageMat.rows / 2;
        InitSpinBox();
    }
}

void CalibrationApp::InitUI() {
    //画像入出力ボタン
    inputButton = new QPushButton("入力画像フォルダ選択", this);
    inputButton->setFixedWidth(170);
    connect(inputButton, SIGNAL (released()), this, SLOT (handleInputButton()));
    outputButton = new QPushButton("出力画像フォルダ選択", this);
    connect(outputButton, SIGNAL (released()), this, SLOT (handleOutputButton()));
    convertButton = new QPushButton("一括変換", this);
    connect(convertButton, SIGNAL (released()), this, SLOT (handleConvertButton()));
    optimiseButton = new QPushButton("最適化", this);
    connect(optimiseButton, SIGNAL (released()), this, SLOT (handleOptimiseButton()));
    mainImage = new QLabel;
    mainImage->setMaximumSize(1200, 500);
    inputLabel = new QLabel;
    outputLabel = new QLabel;

    //レイアウト設定
    QGridLayout *layout1 = new QGridLayout;
    layout1->addWidget(inputButton, 0, 0);
    layout1->addWidget(inputLabel, 0, 1);
    layout1->addWidget(convertButton, 0, 2, Qt::AlignRight);
    layout1->addWidget(outputButton, 1, 0);
    layout1->addWidget(outputLabel, 1, 1);
    layout1->addWidget(optimiseButton, 1, 2, Qt::AlignRight);

    //光学中心x軸用スピンボックス
    QLabel *ocXLabel = new QLabel(tr("optical center x"));
    ocXSpin = new QSpinBox;
    connect(ocXSpin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
        ocx = std::stof(ocXSpin->text().toStdString());
        mainImage->setPixmap(Undistord());
    });

    // 光学中心y軸用スピンボックス
    QLabel *oc_y_label = new QLabel(tr("optical center y"));
    oc_y_spin = new QSpinBox;
    connect(oc_y_spin, QOverload<int>::of(&QSpinBox::valueChanged), [=](int i) {
        ocy = std::stof(oc_y_spin->text().toStdString());
        mainImage->setPixmap(Undistord());
    });


    //focal length用スライダー
    QLineEdit *fl_min_edit = new QLineEdit(tr("500"));
    fl_min_edit->setFixedWidth(38);
    QLabel *fl_min_label = new QLabel(tr("min"));
    QLineEdit *fl_max_edit = new QLineEdit(tr("5000"));
    fl_max_edit->setFixedWidth(38);
    QLabel *fl_max_label = new QLabel(tr("max"));
    fl = calcFocalLength(5000, 500, 49);
    std::string flstr = (boost::format("focal length :  %1%") % fl).str();
    QLabel *focal_length_label = new QLabel(QString::fromStdString(flstr));
    QSlider *focal_length_sld = new QSlider(Qt::Horizontal);
    focal_length_sld->setFocusPolicy(Qt::NoFocus);
    focal_length_sld->setSliderPosition(49);
    focal_length_sld->setMinimumWidth(200);
    connect(focal_length_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        fl = calcFocalLength(std::stof(fl_max_edit->text().toStdString()), std::stof(fl_min_edit->text().toStdString()),
                             value);
        std::string flstr = (boost::format("focal length :  %1%") % fl).str();
        focal_length_label->setText(QString::fromStdString(flstr));
        mainImage->setPixmap(Undistord());
    });

    //レイアウト設定
    QGridLayout *layout2 = new QGridLayout;
    layout2->addWidget(ocXLabel, 0, 0, Qt::AlignCenter);
    layout2->addWidget(ocXSpin, 1, 0);
    layout2->addWidget(oc_y_label, 0, 1, Qt::AlignCenter);
    layout2->addWidget(oc_y_spin, 1, 1);
    layout2->addWidget(fl_min_label, 0, 2, Qt::AlignCenter);
    layout2->addWidget(fl_min_edit, 1, 2);
    layout2->addWidget(focal_length_label, 0, 3, Qt::AlignCenter);
    layout2->addWidget(focal_length_sld, 1, 3);
    layout2->addWidget(fl_max_label, 0, 4, Qt::AlignCenter);
    layout2->addWidget(fl_max_edit, 1, 4);

    //k1用スライダー
    QLineEdit *k1_min_edit = new QLineEdit(tr("-1"));
    k1_min_edit->setFixedWidth(38);
    QLabel *k1_min_label = new QLabel(tr("min"));
    QLineEdit *k1_max_edit = new QLineEdit(tr("1"));
    k1_max_edit->setFixedWidth(38);
    QLabel *k1_max_label = new QLabel(tr("max"));

    k1 = calcDistortionCoefficient(1, -1, 49);
    std::string k1str = (boost::format("k1 :  %1%") % k1).str();
    k1_label = new QLabel(QString::fromStdString(k1str));
    QSlider *k1_sld = new QSlider(Qt::Horizontal);
    k1_sld->setFocusPolicy(Qt::NoFocus);
    k1_sld->setSliderPosition(49);
    k1_sld->setMinimumWidth(200);
    connect(k1_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k1 = calcDistortionCoefficient(std::stof(k1_max_edit->text().toStdString()),
                                       std::stof(k1_min_edit->text().toStdString()), value);
        std::string k1str = (boost::format("k1 :  %1%") % k1).str();
        k1_label->setText(QString::fromStdString(k1str));
        mainImage->setPixmap(Undistord());
    });

    //k2用スライダー
    QLineEdit *k2_min_edit = new QLineEdit(tr("-1"));
    k2_min_edit->setFixedWidth(38);
    QLabel *k2_min_label = new QLabel(tr("min"));
    QLineEdit *k2_max_edit = new QLineEdit(tr("1"));
    k2_max_edit->setFixedWidth(38);
    QLabel *k2_max_label = new QLabel(tr("max"));

    k2 = calcDistortionCoefficient(1, -1, 49);
    std::string k2str = (boost::format("k2 :  %1%") % k2).str();
    k2_label = new QLabel(QString::fromStdString(k2str));
    QSlider *k2_sld = new QSlider(Qt::Horizontal);
    k2_sld->setFocusPolicy(Qt::NoFocus);
    k2_sld->setSliderPosition(49);
    k2_sld->setMinimumWidth(200);
    connect(k2_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k2 = calcDistortionCoefficient(std::stof(k2_max_edit->text().toStdString()),
                                       std::stof(k2_min_edit->text().toStdString()), value);
        std::string k2str = (boost::format("k2 :  %1%") % k2).str();
        k2_label->setText(QString::fromStdString(k2str));
        mainImage->setPixmap(Undistord());
    });

    //p1用スライダー
    QLineEdit *p1_min_edit = new QLineEdit(tr("-1"));
    p1_min_edit->setFixedWidth(38);
    QLabel *p1_min_label = new QLabel(tr("min"));
    QLineEdit *p1_max_edit = new QLineEdit(tr("1"));
    p1_max_edit->setFixedWidth(38);
    QLabel *p1_max_label = new QLabel(tr("max"));

    p1 = calcDistortionCoefficient(1, -1, 49);
    std::string p1str = (boost::format("p1 :  %1%") % p1).str();
    p1_label = new QLabel(QString::fromStdString(p1str));
    QSlider *p1_sld = new QSlider(Qt::Horizontal);
    p1_sld->setFocusPolicy(Qt::NoFocus);
    p1_sld->setSliderPosition(49);
    p1_sld->setMinimumWidth(200);
    connect(p1_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        p1 = calcDistortionCoefficient(std::stof(p1_max_edit->text().toStdString()),
                                       std::stof(p1_min_edit->text().toStdString()), value);
        std::string p1str = (boost::format("p1 :  %1%") % p1).str();
        p1_label->setText(QString::fromStdString(p1str));
        mainImage->setPixmap(Undistord());
    });


    //p2用スライダー
    QLineEdit *p2_min_edit = new QLineEdit(tr("-1"));
    p2_min_edit->setFixedWidth(38);
    QLabel *p2_min_label = new QLabel(tr("min"));
    QLineEdit *p2_max_edit = new QLineEdit(tr("1"));
    p2_max_edit->setFixedWidth(38);
    QLabel *p2_max_label = new QLabel(tr("max"));

    p2 = calcDistortionCoefficient(1, -1, 49);
    std::string p2str = (boost::format("p2 :  %1%") % p2).str();
    p2_label = new QLabel(QString::fromStdString(p2str));
    QSlider *p2_sld = new QSlider(Qt::Horizontal);
    p2_sld->setFocusPolicy(Qt::NoFocus);
    p2_sld->setSliderPosition(49);
    p2_sld->setMinimumWidth(200);
    connect(p2_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        p2 = calcDistortionCoefficient(std::stof(p2_max_edit->text().toStdString()),
                                       std::stof(p2_min_edit->text().toStdString()), value);
        std::string p2str = (boost::format("p2 :  %1%") % p2).str();
        p2_label->setText(QString::fromStdString(p2str));
        mainImage->setPixmap(Undistord());
    });

    //レイアウト設定
    QGridLayout *layout3 = new QGridLayout;
    layout3->addWidget(k1_min_label, 0, 0, Qt::AlignCenter);
    layout3->addWidget(k1_min_edit, 1, 0);
    layout3->addWidget(k1_label, 0, 1, Qt::AlignCenter);
    layout3->addWidget(k1_sld, 1, 1);
    layout3->addWidget(k1_max_label, 0, 2, Qt::AlignCenter);
    layout3->addWidget(k1_max_edit, 1, 2);
    layout3->addWidget(k2_min_label, 0, 3, Qt::AlignCenter);
    layout3->addWidget(k2_min_edit, 1, 3);
    layout3->addWidget(k2_label, 0, 4, Qt::AlignCenter);
    layout3->addWidget(k2_sld, 1, 4);
    layout3->addWidget(k2_max_label, 0, 5, Qt::AlignCenter);
    layout3->addWidget(k2_max_edit, 1, 5);
    layout3->addWidget(p1_min_label, 0, 6, Qt::AlignCenter);
    layout3->addWidget(p1_min_edit, 1, 6);
    layout3->addWidget(p1_label, 0, 7, Qt::AlignCenter);
    layout3->addWidget(p1_sld, 1, 7);
    layout3->addWidget(p1_max_label, 0, 8, Qt::AlignCenter);
    layout3->addWidget(p1_max_edit, 1, 8);
    layout3->addWidget(p2_min_label, 0, 9, Qt::AlignCenter);
    layout3->addWidget(p2_min_edit, 1, 9);
    layout3->addWidget(p2_label, 0, 10, Qt::AlignCenter);
    layout3->addWidget(p2_sld, 1, 10);
    layout3->addWidget(p2_max_label, 0, 11, Qt::AlignCenter);
    layout3->addWidget(p2_max_edit, 1, 11);

    //k3用スライダー
    QLineEdit *k3_min_edit = new QLineEdit(tr("-1"));
    k3_min_edit->setFixedWidth(38);
    QLabel *k3_min_label = new QLabel(tr("min"));
    QLineEdit *k3_max_edit = new QLineEdit(tr("1"));
    k3_max_edit->setFixedWidth(38);
    QLabel *k3_max_label = new QLabel(tr("max"));

    k3 = calcDistortionCoefficient(1, -1, 49);
    std::string k3str = (boost::format("k3 :  %1%") % k3).str();
    k3_label = new QLabel(QString::fromStdString(k3str));
    QSlider *k3_sld = new QSlider(Qt::Horizontal);
    k3_sld->setFocusPolicy(Qt::NoFocus);
    k3_sld->setSliderPosition(49);
    k3_sld->setMinimumWidth(200);
    connect(k3_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k3 = calcDistortionCoefficient(std::stof(k3_max_edit->text().toStdString()),
                                       std::stof(k3_min_edit->text().toStdString()), value);
        std::string k3str = (boost::format("k3 :  %1%") % k3).str();
        k3_label->setText(QString::fromStdString(k3str));
        mainImage->setPixmap(Undistord());
    });

    //k4用スライダー
    QLineEdit *k4_min_edit = new QLineEdit(tr("-1"));
    k4_min_edit->setFixedWidth(38);
    QLabel *k4_min_label = new QLabel(tr("min"));
    QLineEdit *k4_max_edit = new QLineEdit(tr("1"));
    k4_max_edit->setFixedWidth(38);
    QLabel *k4_max_label = new QLabel(tr("max"));

    k4 = calcDistortionCoefficient(1, -1, 49);
    std::string k4str = (boost::format("k4 :  %1%") % k4).str();
    k4_label = new QLabel(QString::fromStdString(k4str));
    QSlider *k4_sld = new QSlider(Qt::Horizontal);
    k4_sld->setFocusPolicy(Qt::NoFocus);
    k4_sld->setSliderPosition(49);
    k4_sld->setMinimumWidth(200);
    connect(k4_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k4 = calcDistortionCoefficient(std::stof(k4_max_edit->text().toStdString()),
                                       std::stof(k4_min_edit->text().toStdString()), value);
        std::string k4str = (boost::format("k4 :  %1%") % k4).str();
        k4_label->setText(QString::fromStdString(k4str));
        mainImage->setPixmap(Undistord());
    });

    //k5用スライダー
    QLineEdit *k5_min_edit = new QLineEdit(tr("-1"));
    k5_min_edit->setFixedWidth(38);
    QLabel *k5_min_label = new QLabel(tr("min"));
    QLineEdit *k5_max_edit = new QLineEdit(tr("1"));
    k5_max_edit->setFixedWidth(38);
    QLabel *k5_max_label = new QLabel(tr("max"));

    k5 = calcDistortionCoefficient(1, -1, 49);
    std::string k5str = (boost::format("k5 :  %1%") % k5).str();
    k5_label = new QLabel(QString::fromStdString(k5str));
    QSlider *k5_sld = new QSlider(Qt::Horizontal);
    k5_sld->setFocusPolicy(Qt::NoFocus);
    k5_sld->setSliderPosition(49);
    k5_sld->setMinimumWidth(200);
    connect(k5_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k5 = calcDistortionCoefficient(std::stof(k5_max_edit->text().toStdString()),
                                       std::stof(k5_min_edit->text().toStdString()), value);
        std::string k5str = (boost::format("k5 :  %1%") % k5).str();
        k5_label->setText(QString::fromStdString(k5str));
        mainImage->setPixmap(Undistord());
    });


    //k6用スライダー
    QLineEdit *k6_min_edit = new QLineEdit(tr("-1"));
    k6_min_edit->setFixedWidth(38);
    QLabel *k6_min_label = new QLabel(tr("min"));
    QLineEdit *k6_max_edit = new QLineEdit(tr("1"));
    k6_max_edit->setFixedWidth(38);
    QLabel *k6_max_label = new QLabel(tr("max"));

    k6 = calcDistortionCoefficient(1, -1, 49);
    std::string k6str = (boost::format("k6 :  %1%") % k6).str();
    k6_label = new QLabel(QString::fromStdString(k6str));
    QSlider *k6_sld = new QSlider(Qt::Horizontal);
    k6_sld->setFocusPolicy(Qt::NoFocus);
    k6_sld->setSliderPosition(49);
    k6_sld->setMinimumWidth(200);
    connect(k6_sld, QOverload<int>::of(&QSlider::valueChanged), [=](int value) {
        k6 = calcDistortionCoefficient(std::stof(k6_max_edit->text().toStdString()),
                                       std::stof(k6_min_edit->text().toStdString()), value);
        std::string k6str = (boost::format("k6 :  %1%") % k6).str();
        k6_label->setText(QString::fromStdString(k6str));
        mainImage->setPixmap(Undistord());
    });

    //レイアウト設定
    QGridLayout *layout4 = new QGridLayout;
    layout4->addWidget(k3_min_label, 0, 0, Qt::AlignCenter);
    layout4->addWidget(k3_min_edit, 1, 0);
    layout4->addWidget(k3_label, 0, 1, Qt::AlignCenter);
    layout4->addWidget(k3_sld, 1, 1);
    layout4->addWidget(k3_max_label, 0, 2, Qt::AlignCenter);
    layout4->addWidget(k3_max_edit, 1, 2);
    layout4->addWidget(k4_min_label, 0, 3, Qt::AlignCenter);
    layout4->addWidget(k4_min_edit, 1, 3);
    layout4->addWidget(k4_label, 0, 4, Qt::AlignCenter);
    layout4->addWidget(k4_sld, 1, 4);
    layout4->addWidget(k4_max_label, 0, 5, Qt::AlignCenter);
    layout4->addWidget(k4_max_edit, 1, 5);
    layout4->addWidget(k5_min_label, 0, 6, Qt::AlignCenter);
    layout4->addWidget(k5_min_edit, 1, 6);
    layout4->addWidget(k5_label, 0, 7, Qt::AlignCenter);
    layout4->addWidget(k5_sld, 1, 7);
    layout4->addWidget(k5_max_label, 0, 8, Qt::AlignCenter);
    layout4->addWidget(k5_max_edit, 1, 8);
    layout4->addWidget(k6_min_label, 0, 9, Qt::AlignCenter);
    layout4->addWidget(k6_min_edit, 1, 9);
    layout4->addWidget(k6_label, 0, 10, Qt::AlignCenter);
    layout4->addWidget(k6_sld, 1, 10);
    layout4->addWidget(k6_max_label, 0, 11, Qt::AlignCenter);
    layout4->addWidget(k6_max_edit, 1, 11);

    //レイアウト設定
    QVBoxLayout *vbox = new QVBoxLayout;
    vbox->addLayout(layout1);
    vbox->addLayout(layout2);
    vbox->addLayout(layout3);
    vbox->addLayout(layout4);
    QGridLayout *layout5 = new QGridLayout;
    layout5->addWidget(mainImage, 0, 0, Qt::AlignCenter);
    vbox->addLayout(layout5);
    setLayout(vbox);
    setWindowTitle("Camera Calibration");
}

void CalibrationApp::handleInputButton() {
    QString dirPath = QFileDialog::getExistingDirectory(this);
    if (dirPath.size() == 0) {
        return;
    }
    std::string path = dirPath.toStdString();
    inputFiles.clear();
    SearchDir(dirPath.toStdString(), inputFiles);
    imageMat = cv::imread(inputFiles[0]);
    mainImage->setPixmap(Mat2Pixmap(imageMat));
    inputLabel->setText(dirPath);
    ocx = imageMat.cols / 2;
    ocy = imageMat.rows / 2;
    InitSpinBox();
}

void CalibrationApp::handleOutputButton() {
    QString dirPath = QFileDialog::getExistingDirectory(this);
    if (dirPath.size() == 0) {
        return;
    }
    outputPath = dirPath.toStdString();
    outputLabel->setText(dirPath);
}

void CalibrationApp::handleConvertButton() {
    if (outputPath.empty()) {
        return;
    }
/*    QProgressBar *bar = new QProgressBar;
    bar->setRange(0,inputFiles.size());
    bar->setValue( 0 );
    QDialog *dialog = new QDialog;
    QGridLayout *layout = new QGridLayout;
    layout->addWidget(bar, 0, 0, Qt::AlignCenter);
    dialog->setLayout(layout);
    dialog->exec();*/
    outputPath.push_back('/');
    for (int i = 0; i < inputFiles.size(); i++) {
        cv::Mat image = cv::imread(inputFiles[i]);
        cv::Mat undistorted;
        double cameraArray[] = {fl, 0, (float) ocx, 0, fl, (float) ocy, 0, 0, 1};
        cv::Mat mtx(3, 3, CV_64FC1, cameraArray);
        double paramArray[] = {k1, k2, p1, p2, k3, k4, k5, k6};
        cv::Mat dist(8, 1, CV_64FC1, paramArray);
        undistort(image, undistorted, mtx, dist);
        int path_i = inputFiles[i].find_last_of("/") + 1;
        int ext_i = inputFiles[i].find_last_of(".");
        std::string extname = inputFiles[i].substr(ext_i, inputFiles[i].size() - ext_i);
        std::string filename = inputFiles[i].substr(path_i, ext_i - path_i);
        cv::imwrite(outputPath + filename + extname, undistorted);
        //bar->setValue(i+1);
    }
/*    bar->close();
    dialog->close();
    delete dialog;*/
}

void CalibrationApp::handleOptimiseButton() {
    if (inputFiles.size() < 2) {
        return;
    }
    cv::Mat imageMat2 = cv::imread(inputFiles[1]);
    double k[] = {k1, k2, k3, k4, k5, k6};
    double p[] = {p1, p2};
    double camera[] = {fl, (double)ocx, (double)ocy};
    CameraCalibration(imageMat, imageMat2, k, p, camera);
    k1 = k[0];
    std::string str = (boost::format("k1 :  %1%") % k1).str();
    k1_label->setText(QString::fromStdString(str));
    k2 = k[1];
    str = (boost::format("k2 :  %1%") % k2).str();
    k2_label->setText(QString::fromStdString(str));
    k3 = k[2];
    str = (boost::format("k3 :  %1%") % k3).str();
    k3_label->setText(QString::fromStdString(str));
    k4 = k[3];
    str = (boost::format("k4 :  %1%") % k4).str();
    k4_label->setText(QString::fromStdString(str));
    k5 = k[4];
    str = (boost::format("k5 :  %1%") % k5).str();
    k5_label->setText(QString::fromStdString(str));
    k6 = k[5];
    str = (boost::format("k6 :  %1%") % k6).str();
    k6_label->setText(QString::fromStdString(str));
    p1 = p[0];
    str = (boost::format("p1 :  %1%") % p1).str();
    p1_label->setText(QString::fromStdString(str));
    p2 = p[1];
    str = (boost::format("p2 :  %1%") % p2).str();
    p2_label->setText(QString::fromStdString(str));

    mainImage->setPixmap(Undistord());
}


QPixmap CalibrationApp::Mat2Pixmap(cv::Mat &image) {
    cv::Mat converted;
    cv::cvtColor(image, converted, CV_BGR2RGB);
    QImage qImage = QImage((const unsigned char *) (converted.data),
                           converted.cols, converted.rows, QImage::Format_RGB888);
    return QPixmap::fromImage(qImage).scaled(1200, 500, Qt::KeepAspectRatio);
}

QPixmap CalibrationApp::Undistord() {
    cv::Mat undistorted;
    double cameraArray[] = {fl, 0, (float) ocx, 0, fl, (float) ocy, 0, 0, 1};
    cv::Mat mtx(3, 3, CV_64FC1, cameraArray);
    double paramArray[] = {k1, k2, p1, p2, k3, k4, k5, k6};
    cv::Mat dist(8, 1, CV_64FC1, paramArray);
    undistort(imageMat, undistorted, mtx, dist);
    return Mat2Pixmap(undistorted);
}

float CalibrationApp::calcFocalLength(float max_value, float min_value, float value) {
    return min_value + (max_value - min_value) * pow((value / 99), 2);
}


float CalibrationApp::calcDistortionCoefficient(float max_value, float min_value, float value) {
    float dc;
    if (value < 49) {
        dc = min_value * pow(((value - 49) / 49), 2);
    } else {
        dc = max_value * pow(((value - 49) / 49), 2);
    }
    return dc;
}

void CalibrationApp::InitSpinBox() {
    ocXSpin->setMinimum(0);
    ocXSpin->setMaximum(imageMat.cols - 1);
    ocXSpin->setValue(imageMat.cols / 2);
    oc_y_spin->setMinimum(0);
    oc_y_spin->setMaximum(imageMat.rows - 1);
    oc_y_spin->setValue(imageMat.rows / 2);

}
