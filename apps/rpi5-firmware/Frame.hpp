#pragma once

#include <opencv4/opencv2/core.hpp>

#define RED cv::Scalar(0,0,255)
#define GREEN cv::Scalar(0,255,0)
#define BLUE cv::Scalar(255,0,0)
#define YELLOW cv::Scalar(0, 255, 255)

class Frame {
public:

	Frame();
	Frame(const cv::Mat &frameRaw);
	~Frame();

	Frame &operator=(const Frame &frame);

	void warp(cv::Mat matrix);
	void open();
	void close();
	void canny();
	void setPointValue(const int x, const int y, const uchar val);
	uchar getPointValue(const int x, const int y);
	void cropp(const cv::Rect &rect);
	void histogram(std::vector<int> &histogram);
	void transformToBinary();
	Frame getColoredFrame();
	void drawLine(cv::Point &pt1, cv::Point &pt2, const cv::Scalar &color, const int thickness);
	void save(const std::string &filename);
	void showInScreen(const std::string &winName);
	int getHeight() const;
	int getWidth() const;
	cv::Mat &getRawFrame();

private:

	cv::Mat _frameRaw;
	cv::Mat _kernel;
};	

std::ostream &operator<<(std::ostream &stream, const Frame &frame);
