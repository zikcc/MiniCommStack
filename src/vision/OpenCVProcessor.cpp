#include "vision/OpenCVProcessor.hpp"
#include "opencv2/core/mat.hpp"

OpenCVProcessor::OpenCVProcessor(const std::string& output_path, PixelFormat fmt,  unsigned width, unsigned height) : pixel_format_(fmt), width_(width), height_(height){
    const int fourcc = cv::VideoWriter::fourcc('a','v','c','1');
    const double fps = 30.0;
    const cv::Size frame_size(width, height);
    
    writer_.open(output_path, fourcc, fps, frame_size);
    if (!writer_.isOpened()) {
        throw std::runtime_error("Failed to initialize video writer");
    }
}

OpenCVProcessor::~OpenCVProcessor() {
    if (writer_.isOpened()) writer_.release();
}

void OpenCVProcessor::process_frame(const std::vector<uint8_t>& raw_data) {
    cv:: Mat frame;
    if (pixel_format_ == PixelFormat::MJPEG) {
        // MJPEG解码
        cv::Mat raw_mat(1, raw_data.size(), CV_8UC1, (void*)raw_data.data());
        cv::Mat frame = cv::imdecode(raw_mat, cv::IMREAD_COLOR);
    } else if (pixel_format_ == PixelFormat::YUYV) {
        // 将 YUYV 数据转换为 Mat
        cv::Mat yuyv(height_, width_, CV_8UC2, (void*)raw_data.data());

        // 转换 YUYV → BGR
        cv::cvtColor(yuyv, frame, cv::COLOR_YUV2BGR_YUYV);
    }
    
    if (!frame.empty()) {
        // 转换为 RGB
        cv::cvtColor(frame, current_frame_, cv::COLOR_BGR2RGB);
        apply_algorithm(current_frame_);
        frame_count_++;
    }
}

void OpenCVProcessor::apply_algorithm(cv::Mat& frame) {
    // 示例算法：Canny边缘检测
    cv::Mat gray, edges;
    cv::cvtColor(frame, gray, cv::COLOR_RGB2GRAY);
    cv::Canny(gray, edges, 100, 200);
    cv::cvtColor(edges, frame, cv::COLOR_GRAY2RGB);
}

void OpenCVProcessor::save_output() {
    if (writer_.isOpened() && !current_frame_.empty()) {
        writer_.write(current_frame_);
    }
}