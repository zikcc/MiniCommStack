#pragma once
#include <opencv2/opencv.hpp>
#include <string>

class OpenCVProcessor {
public:
    enum class PixelFormat { MJPEG, YUYV };
    explicit OpenCVProcessor(const std::string& output_path, PixelFormat fmt, unsigned width, unsigned height);
    ~OpenCVProcessor();
    
    void process_frame(const std::vector<uint8_t>& raw_data);
    void save_output();
    
private:
    cv::VideoWriter writer_;
    cv::Mat current_frame_;
    unsigned frame_count_ = 0;
    PixelFormat pixel_format_;
    unsigned width_;
    unsigned height_;
    void apply_algorithm(cv::Mat& frame);
};