#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>
#include <sstream>
#include <string>
#include <iomanip>
#include <chrono>
#include <ctime>

std::string generateTimestampedFilename() {
    auto now = std::chrono::system_clock::now();
    std::time_t now_time = std::chrono::system_clock::to_time_t(now);
    std::tm tm = *std::localtime(&now_time);
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y%m%d_%H%M%S") << ".png";
    return oss.str();
}

int main(int argc, char **argv) {
    std::cout << R"(keymap:
    z x | scale image + -
     m  | cycle through Colormaps
     p  | save frame to file
    r t | record / stop (Not Implemented!)
     q  | quit
     )" << std::endl;
    int mapInt = 0;
    int scale = 2;
    bool tempConv = true;
    bool recording = false;
    std::vector<int> colormaps = {
        cv::ColormapTypes::COLORMAP_BONE,
        cv::ColormapTypes::COLORMAP_JET,
        cv::ColormapTypes::COLORMAP_DEEPGREEN,
        cv::ColormapTypes::COLORMAP_VIRIDIS,
        cv::ColormapTypes::COLORMAP_CIVIDIS,
        cv::ColormapTypes::COLORMAP_TURBO,
        cv::ColormapTypes::COLORMAP_TWILIGHT_SHIFTED,
        cv::ColormapTypes::COLORMAP_OCEAN,
        cv::ColormapTypes::COLORMAP_PINK,
        cv::ColormapTypes::COLORMAP_HOT,
        cv::ColormapTypes::COLORMAP_MAGMA,
        cv::ColormapTypes::COLORMAP_INFERNO,
        //cv::ColormapTypes::COLORMAP_HSV,
        //cv::ColormapTypes::COLORMAP_AUTUMN,
        //cv::ColormapTypes::COLORMAP_WINTER,
        //cv::ColormapTypes::COLORMAP_RAINBOW,
        //cv::ColormapTypes::COLORMAP_SUMMER,
        //cv::ColormapTypes::COLORMAP_SPRING,
        //cv::ColormapTypes::COLORMAP_COOL,
        //cv::ColormapTypes::COLORMAP_PARULA,
        //cv::ColormapTypes::COLORMAP_PLASMA,
        //cv::ColormapTypes::COLORMAP_TWILIGHT,
    };
    int colormapsLen = static_cast<int>(colormaps.size());
    int deviceInt{std::stoi(argv[1])};
    std::cout << "Opening device: " << deviceInt << std::endl;
    cv::VideoCapture cap(deviceInt);
    if (!cap.isOpened()) {
        std::cerr << "Error: Could not open the webcam." << std::endl;
        return -1;
    }
    cap.set(cv::CAP_PROP_CONVERT_RGB, false);
    cv::Mat frame(cv::Size(256, 384), CV_16UC2);
    while (true) {
        cap >> frame;
        if (frame.empty()) {
            std::cerr << "Error: Could not grab a frame." << std::endl;
            break;
        }
        cv::Rect topHalf(0, 0, frame.cols, frame.rows / 2);
        cv::Mat visibleMat = frame(topHalf);
        cv::Rect bottomHalf(0, frame.rows / 2 , frame.cols, frame.rows / 2);
        cv::Mat thermalMat(cv::Size(256, 192), CV_16UC2);
        thermalMat = frame(bottomHalf);
        int offset = 32;
        cv::Vec2w centerPixel = thermalMat.at<cv::Vec2w>(128-offset, 96-offset);
        uint16_t thermValueLow = centerPixel[0];
        uint16_t thermValueHigh = centerPixel[1];
        std::string tempFormat;
        float vTemp;
        float thermValue = (thermValueLow + thermValueHigh) / 2;
        float cTemp = thermValue / 64 - 273.15;
        if (tempConv) {
            tempFormat = " F";
            vTemp = (cTemp * 9 / 5) + 32;
        } else {
            tempFormat = " C";
            vTemp = cTemp;
        }
        cv::Mat matRGB;
        cv::cvtColor(visibleMat, matRGB, cv::COLOR_YUV2BGR_YUYV);
        cv::Mat colormapped;
        cv::applyColorMap(matRGB, colormapped, colormaps[mapInt]);
        cv::Mat scaledImage;
        cv::resize(colormapped, scaledImage, cv::Size(256 * scale, 192 * scale), 0, 0, cv::INTER_NEAREST);
        std::ostringstream oss;
        oss.precision(2);
        oss << std::fixed << vTemp;
        std::string tempText = oss.str() + tempFormat;
        cv::putText(scaledImage, tempText, cv::Point(5, 10), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(0, 0, 0), 2);
        cv::putText(scaledImage, tempText, cv::Point(5, 10), cv::FONT_HERSHEY_SIMPLEX, 0.4, cv::Scalar(255, 255, 255), 1);
        cv::circle(scaledImage, cv::Point(128 * scale, 96 * scale), 1, cv::Scalar(0, 0, 0), 2);
        cv::circle(scaledImage, cv::Point(128 * scale, 96 * scale), 1, cv::Scalar(255, 255, 255), 1);
        cv::imshow("Webcam", scaledImage);
        switch (cv::waitKey(30)) {
            case 'q':
                return 0;
                break;
            case 'm':
                if (mapInt == colormapsLen-1) {
                    mapInt = 0;
                } else {
                    mapInt++;
                }
                break;
            case 'z':
                scale++;
                break;
            case 'x':
                if (scale > 1) {
                    scale--;
                }
                break;
            case 'p':
                if (!cv::imwrite(generateTimestampedFilename(), scaledImage)) {
                        std::cerr << "Could not save image!" << std::endl;
                        return -1;
                    }
                break;
            case 'w':
                if (tempConv) {
                    tempConv = false;
                } else {
                    tempConv = true;
                }
                break;
            case 'r':
                recording = true;
                std::cerr << "Not Implemented!" << std::endl;
                break;
            case 't':
                recording = false;
                std::cerr << "Not Implemented!" << std::endl;
                break;
        }
    }
    cap.release();
    cv::destroyAllWindows();
    return 0;
}
