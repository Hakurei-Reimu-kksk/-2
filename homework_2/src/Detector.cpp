#include "Detector.hpp"
#include <iostream>
Detector::Detector(const std::string& configPath) 
{
    m_dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_APRILTAG_36h11);
#if CV_VERSION_MAJOR>=4 && CV_VERSION_MINOR>=7
    m_detectorParams=cv::makePtr<cv::aruco::DetectorParameters>();
#else
    m_detectorParams=cv::aruco::DetectorParameters::create();
#endif
    m_binaryThreshold=128;
    m_hMin=0; m_hMax=180;
    m_sMin=100; m_sMax=255;
    m_vMin=100; m_vMax=255;
    if (!configPath.empty()) 
    {
        loadConfig(configPath);
    }
}
void Detector::loadConfig(const std::string& configPath) 
{
    cv::FileStorage fs(configPath, cv::FileStorage::READ);
    if (!fs.isOpened()) 
    {
        std::cerr<<"警告：无法打开配置文件："<<configPath 
                <<"使用默认参数"<<std::endl;
        return;
    }
    fs["binary_threshold"]>>m_binaryThreshold;
    fs["hsv_min"]>>m_hMin; fs["hsv_max"]>>m_hMax;
    fs["sat_min"]>>m_sMin; fs["sat_max"]>>m_sMax;
    fs["val_min"]>>m_vMin; fs["val_max"]>>m_vMax;
    fs.release();
    std::cout<<"成功加载配置文件： "<<configPath<<std::endl;
}
void Detector::detect(const cv::Mat& src) 
{
    if (src.empty()) return;
    m_image_raw=src.clone();
    m_result=src.clone();
    cv::Mat gray;
    cv::cvtColor(src, gray, cv::COLOR_BGR2GRAY);
    cv::Mat binaryInv;
    cv::threshold(gray, binaryInv, 100, 255, cv::THRESH_BINARY_INV);
    cv::threshold(gray, m_image_binary, 100, 255, cv::THRESH_BINARY);
    cv::cvtColor(src, m_image_hsv, cv::COLOR_BGR2HSV);
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(25, 25));
    cv::Mat morphed;
    cv::morphologyEx(binaryInv, morphed, cv::MORPH_OPEN, kernel);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(morphed, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    int tagCount = 0;
    for (const auto& contour : contours) 
    {
        double area = cv::contourArea(contour);
        if (area<3000 || area>200000) continue; 
        cv::RotatedRect minRect=cv::minAreaRect(contour);
        float width = minRect.size.width;
        float height = minRect.size.height;
        float aspectRatio = width / height;
        if (aspectRatio > 0.8 && aspectRatio<1.2) 
        {
            tagCount++;
            cv::Point2f vertices[4];
            minRect.points(vertices);
            for (int i=0; i<4; i++) 
            {
                cv::line(m_result, vertices[i], vertices[(i+1) % 4], cv::Scalar(0, 255, 0), 3);
            }
            cv::circle(m_result, minRect.center, 8, cv::Scalar(0, 0, 255), -1);
            cv::putText(m_result, "Tag"+std::to_string(tagCount), 
                        cv::Point(minRect.center.x-30, minRect.center.y-20),
                        cv::FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(255, 0, 0), 2);
        }
    }
    std::cout<< "成功检测到"<<tagCount<<"个标签"<<std::endl;
}
void Detector::detect(cv::VideoCapture& cap) 
{
    cv::Mat frame;
    cap>>frame;
    if (frame.empty()) return;
    m_image_raw=frame.clone();
    m_result=frame.clone();
    cv::cvtColor(frame, m_image_hsv, cv::COLOR_BGR2HSV);
    cv::Mat gray;
    cv::cvtColor(frame, gray, cv::COLOR_BGR2GRAY);
    cv::threshold(gray, m_image_binary, m_binaryThreshold, 255, cv::THRESH_BINARY);
    processColor(m_image_hsv, cv::Scalar(0, m_sMin, m_vMin), cv::Scalar(10, m_sMax, m_vMax), cv::Scalar(0, 0, 255), "Red");
    processColor(m_image_hsv, cv::Scalar(160, m_sMin, m_vMin), cv::Scalar(180, m_sMax, m_vMax), cv::Scalar(0, 0, 255), "Red");
    processColor(m_image_hsv, cv::Scalar(35, m_sMin, m_vMin), cv::Scalar(85, m_sMax, m_vMax), cv::Scalar(0, 255, 0), "Green");
    processColor(m_image_hsv, cv::Scalar(90, m_sMin, m_vMin), cv::Scalar(150, m_sMax, m_vMax), cv::Scalar(255, 0, 0), "Blue");
    processColor(m_image_hsv, cv::Scalar(20, m_sMin, m_vMin), cv::Scalar(35, m_sMax, m_vMax), cv::Scalar(0, 255, 255), "Yellow");
    processColor(m_image_hsv, cv::Scalar(11, m_sMin, m_vMin), cv::Scalar(19, m_sMax, m_vMax), cv::Scalar(0, 165, 255), "Orange");
    processColor(m_image_hsv, cv::Scalar(0, 0, 140), cv::Scalar(180, 60, 255), cv::Scalar(255, 255, 255), "White");
}
void Detector::processColor(const cv::Mat& hsv, const cv::Scalar& low, const cv::Scalar& high, 
                            const cv::Scalar& drawColor, const std::string& label) {
    cv::Mat mask;
    cv::inRange(hsv, low, high, mask);
    cv::Mat kernel=cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
    cv::morphologyEx(mask, mask, cv::MORPH_OPEN, kernel);
    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(mask, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);
    for (size_t i=0; i<contours.size(); i++) 
    {
        double area=cv::contourArea(contours[i]);
        if (area>300 && area<15000) 
        { 
            cv::RotatedRect minRect=cv::minAreaRect(contours[i]);           
            float width=minRect.size.width;
            float height=minRect.size.height;
            if (width==0 || height==0) continue;
            float aspectRatio=width / height;
            if (aspectRatio>1.0f) aspectRatio=1.0f/aspectRatio;
            if (aspectRatio>0.6f) 
            {
                float fillRatio=area/(width*height);              
                if (fillRatio>0.65f) 
                {
                    cv::Rect rect=cv::boundingRect(contours[i]);                   
                    cv::rectangle(m_result, rect, drawColor, 2);                  
                    int baseLine=0;
                    cv::Size labelSize=cv::getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
                    cv::putText(m_result, label, 
                                cv::Point(rect.x, rect.y-5), 
                                cv::FONT_HERSHEY_SIMPLEX, 0.5, drawColor, 1);
                }
            }
        }
    }
}
void Detector::updateHSVParams() {
}
