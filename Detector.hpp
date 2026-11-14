#pragma once
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <string>
#include <vector>
class Detector 
{
public:
    Detector(const std::string& configPath="");
    ~Detector()=default;
    void detect(const cv::Mat& src);
    void detect(cv::VideoCapture& cap);
    cv::Mat getRawImage() const {return m_image_raw;}
    cv::Mat getBinaryImage() const {return m_image_binary;}
    cv::Mat getHsvImage() const {return m_image_hsv;}
    cv::Mat getResultImage() const {return m_result;}
    int m_binaryThreshold;
    int m_hMin, m_hMax, m_sMin, m_sMax, m_vMin, m_vMax;
    void updateHSVParams();
private:
    void loadConfig(const std::string& configPath);
    void processColor(const cv::Mat& hsv, const cv::Scalar& low, const cv::Scalar& high, 
                      const cv::Scalar& drawColor, const std::string& label);
    cv::Mat m_image_raw;
    cv::Mat m_image_binary;
    cv::Mat m_image_hsv;
    cv::Mat m_result;
    cv::Ptr<cv::aruco::Dictionary> m_dictionary;
    cv::Ptr<cv::aruco::DetectorParameters> m_detectorParams;
};