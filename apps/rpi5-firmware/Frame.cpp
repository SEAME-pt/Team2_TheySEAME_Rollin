#include "Frame.hpp"
#include <opencv4/opencv2/imgproc.hpp>
#include <opencv4/opencv2/imgcodecs.hpp>
#include <opencv4/opencv2/highgui.hpp>

Frame::Frame() {
	_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
}

Frame::Frame(const cv::Mat &frameRaw) {
	_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	_frameRaw = frameRaw.clone();
}

Frame::Frame(const int height, const int width, const int type) {
	_kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(3, 3));
	_frameRaw = cv::Mat(height, width, type);
}

Frame::~Frame() {}

Frame &Frame::operator=(const Frame &frame) {
	if (this != &frame) {
		_frameRaw = frame._frameRaw.clone();
	}
	return (*this);
}

Frame &Frame::operator*(const int mult) {
	_frameRaw *= 255;
	return (*this);
}

void Frame::warp(cv::Mat matrix) {
	cv::warpPerspective(_frameRaw, _frameRaw, matrix, _frameRaw.size());
}

void Frame::open() {
	cv::morphologyEx(_frameRaw, _frameRaw, cv::MORPH_OPEN, _kernel);
}

void Frame::close() {
	cv::morphologyEx(_frameRaw, _frameRaw, cv::MORPH_CLOSE, _kernel);
}

void Frame::canny() {
	cv::Mat edges;
	cv::Canny(_frameRaw, edges, 50, 200);
	_frameRaw = edges;
}

void Frame::setPointValue(const int x, const int y, const uchar val) {
	_frameRaw.at<uchar>(y, x) = val;
}

uchar Frame::getPointValue(const int x, const int y) {
	return (_frameRaw.at<uchar>(y, x));
}

void Frame::cropp(const cv::Rect &rect) {
	_frameRaw = _frameRaw(rect);
}

void Frame::histogram(std::vector<int> &histogram) {
	cv::Mat lowerHalf = _frameRaw(cv::Range(_frameRaw.rows - (_frameRaw.rows / 2), _frameRaw.rows), cv::Range::all());

	cv::reduce(lowerHalf, histogram, 0, cv::REDUCE_SUM, CV_32S);
}

void Frame::transformToBinary(const int thresh) {
	cv::cvtColor(_frameRaw, _frameRaw, cv::COLOR_BGR2GRAY);
	cv::threshold(_frameRaw, _frameRaw, 1, 255, cv::THRESH_BINARY);
}

Frame Frame::getColoredFrame() {
	Frame copyFrame;

	cv::cvtColor(_frameRaw, copyFrame.getMatObj(), cv::COLOR_GRAY2BGR);
	return (copyFrame);
}

void Frame::drawLine(cv::Point &pt1, cv::Point &pt2, const cv::Scalar &color, const int thickness) {
	cv::line(_frameRaw, pt1, pt2, color, thickness);
}

void Frame::save(const std::string &filename) {
	cv::imwrite("./camera/" + filename, _frameRaw);
}

void Frame::combineFrames(const Frame &frame) {
	cv::hconcat(_frameRaw, frame._frameRaw, _frameRaw);
}

void Frame::showInScreen(const std::string &winName) {
	cv::Mat resized;
	
	cv::resize(_frameRaw, resized, cv::Size(this->getWidth(), 300));
	cv::imshow(winName, resized);
	cv::waitKey(1);
}

int Frame::getHeight() const { return (_frameRaw.rows); }

int Frame::getWidth() const { return (_frameRaw.cols); }

void *Frame::getRawData() { return (_frameRaw.data); }

cv::Mat &Frame::getMatObj() { return (_frameRaw); }

std::ostream &operator<<(std::ostream &os, const Frame &frame) {
	os << "Frame: " << frame.getHeight() << " " << frame.getWidth();
	return (os);
}
