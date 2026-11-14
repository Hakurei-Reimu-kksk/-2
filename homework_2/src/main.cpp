#include "Detector.hpp"
#include <iostream>
#include <opencv2/opencv.hpp>
Detector* g_detector=nullptr;
void onTrackbar(int, void*) 
{
    if (!g_detector) return;
    int sMax = cv::getTrackbarPos("S Max", "result (Cube)");
    if (sMax > 0) 
    {
        g_detector->m_binaryThreshold=cv::getTrackbarPos("Binary Thresh", "result (Cube)");
        g_detector->m_hMin=cv::getTrackbarPos("H Min", "result (Cube)");
        g_detector->m_hMax=cv::getTrackbarPos("H Max", "result (Cube)");
        g_detector->m_sMin=cv::getTrackbarPos("S Min", "result (Cube)");
        g_detector->m_sMax=cv::getTrackbarPos("S Max", "result (Cube)");
        g_detector->m_vMin=cv::getTrackbarPos("V Min", "result (Cube)");
        g_detector->m_vMax=cv::getTrackbarPos("V Max", "result (Cube)");
    }
}
void createUI(const std::string& winName, Detector* detector) 
{
    g_detector = detector;
    cv::createTrackbar("Binary Thresh", winName, nullptr, 255, onTrackbar);
    cv::createTrackbar("H Min", winName, nullptr, 180, onTrackbar);
    cv::createTrackbar("H Max", winName, nullptr, 180, onTrackbar);
    cv::createTrackbar("S Min", winName, nullptr, 255, onTrackbar);
    cv::createTrackbar("S Max", winName, nullptr, 255, onTrackbar);
    cv::createTrackbar("V Min", winName, nullptr, 255, onTrackbar);
    cv::createTrackbar("V Max", winName, nullptr, 255, onTrackbar);
    cv::setTrackbarPos("Binary Thresh", winName, 128);
    cv::setTrackbarPos("H Min", winName, 0);
    cv::setTrackbarPos("H Max", winName, 180);
    cv::setTrackbarPos("S Min", winName, 100);
    cv::setTrackbarPos("S Max", winName, 255);
    cv::setTrackbarPos("V Min", winName, 100); 
    cv::setTrackbarPos("V Max", winName, 255); 
    onTrackbar(0, 0);
}
int main(int argc, char** argv) 
{
    Detector detector("config.yaml");
    std::cout<<"任务1：AprilTag检测"<<std::endl;
    cv::Mat tagImg = cv::imread("tag_image.jpg");
    if (!tagImg.empty()) 
    {
        detector.detect(tagImg);
        cv::imshow("Image_raw (AprilTag)", detector.getRawImage());
        cv::imshow("Image_binary (AprilTag)", detector.getBinaryImage());
        cv::imshow("Image_hsv (AprilTag)", detector.getHsvImage());
        cv::imshow("result (AprilTag)", detector.getResultImage());
        std::cout<<"按任意键继续执行任务2"<<std::endl;
        cv::waitKey(0);
    } else {
        std::cerr<<"无法找到'tag_image.jpg'"<<std::endl;
    }
    std::cout<<"任务2：魔方颜色检测"<<std::endl;
    cv::VideoCapture cap("cube_video.mp4");
    if (!cap.isOpened()) 
    {
        std::cerr<<"无法打开cube_video.mp4，尝试打开摄像头"<<std::endl;
        cap.open(0); 
    }
    if (cap.isOpened()) 
    {
        std::string resultWinName="result (Cube)";
        cv::namedWindow(resultWinName, cv::WINDOW_AUTOSIZE);
        createUI(resultWinName, &detector);
        while (true) 
        {
            detector.detect(cap);
            cv::Mat raw = detector.getRawImage();
            if (raw.empty()) break; 
            cv::imshow("Image_raw (Cube)", raw);
            cv::imshow("Image_binary (Cube)", detector.getBinaryImage());
            cv::imshow("Image_hsv (Cube)", detector.getHsvImage());
            cv::imshow(resultWinName, detector.getResultImage());
            int key=cv::waitKey(30);
            if (key=='q' || key==27) break;
        }
    } else 
    {
        std::cerr<<"无法打开视频源"<<std::endl;   
    }
    cv::destroyAllWindows();
    return 0;
}
