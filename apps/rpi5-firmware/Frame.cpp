#include "Frame.hpp"
#include <opencv4/opencv2/imgproc.hpp>

Frame::Frame() {
	_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
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
	_frameRaw = _frameRaw(cv::Rect(0, 864 - _frameRaw.rows, _frameRaw.cols, _frameRaw.rows));
}

void Frame::transformToBinary() {
	cv::cvtColor(_frameRaw, _frameRaw, cv::COLOR_BGR2GRAY);
	cv::threshold(_frameRaw, _frameRaw, 127, 255, cv::THRESH_BINARY);
}

int Frame::getHeight() { return (_frameRaw.rows); }

int Frame::getWidth() { return (_frameRaw.cols); }
