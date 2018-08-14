cmake_minimum_required(VERSION 3.12)
project(CameraCalibration)

set(CMAKE_CXX_STANDARD 11)
# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

add_executable(CameraCalibration
        src/main.cpp
        src/optical_flow/feature_detection.cpp
        src/optical_flow/feature_detection.hpp
        src/optical_flow/optical_flow.cpp
        src/optical_flow/optical_flow.hpp
        src/image/image_io.cpp src/image/image_io.hpp
        src/geometry/geometry.cpp src/geometry/geometry.hpp
        src/optimisation/camera_calibration.cpp
        src/optimisation/camera_calibration.hpp
        src/gui/main_window.cpp
        src/gui/main_window.hpp)

find_package(OpenCV 3.4 REQUIRED)
include_directories( ${OpenCV_INCLUDE_DIRS} )
target_link_libraries(CameraCalibration ${OpenCV_LIBS})
find_package(Ceres REQUIRED)
include_directories(${CERES_INCLUDE_DIRS})
target_link_libraries(CameraCalibration ${CERES_LIBRARIES})

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
target_link_libraries(CameraCalibration Qt5::Widgets)



