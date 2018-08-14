#include "src/gui/main_window.hpp"
#include <QApplication>



int main(int argc, char **argv) {
    QApplication app(argc, argv);

    CalibrationApp window;
    window.show();

    return app.exec();
}