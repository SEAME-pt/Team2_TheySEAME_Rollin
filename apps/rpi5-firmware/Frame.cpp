#include "Frame.hpp"
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>

Frame::Frame() {
	_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
}

Frame::Frame(const cv::Mat &frameRaw) {
	_frameRaw = frameRaw;
}

Frame::~Frame() {}

void Frame::warp(cv::Mat matrix) {
	cv::warpPerspective(_frameRaw, _frameRaw, matrix, _frameRaw.size());
}

void Frame::open() {
	cv::erode(_frameRaw, _frameRaw, _kernel);
	cv::dilate(_frameRaw, _frameRaw, _kernel);
	cv::medianBlur(_frameRaw, _frameRaw, 9);
}

void Frame::close() {
	cv::dilate(_frameRaw, _frameRaw, _kernel);
	cv::erode(_frameRaw, _frameRaw, _kernel);
	cv::medianBlur(_frameRaw, _frameRaw, 9);
}

void Frame::cropp() {
	_frameRaw = _frameRaw(cv::Rect(0, _frameRaw.rows - 464, _frameRaw.cols, 464));
}

void Frame::transformToBinary() {
	cv::cvtColor(_frameRaw, _frameRaw, cv::COLOR_BGR2GRAY);
	cv::threshold(_frameRaw, _frameRaw, 127, 255, cv::THRESH_BINARY);
}

void Frame::save(const std::string &filename) {
	cv::imwrite(filename, _frameRaw);
}

int Frame::getHeight() const { return (_frameRaw.rows); }

int Frame::getWidth() const { return (_frameRaw.cols); }

cv::Mat &Frame::getRawFrame() { return (_frameRaw); }

std::ostream &operator<<(std::ostream &os, const Frame &frame) {
	os << "Frame: " << frame.getHeight() << " " << frame.getWidth();
	return (os);
}
