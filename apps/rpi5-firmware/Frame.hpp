#pragma once

#include <opencv4/opencv2/core.hpp>

#define RED cv::Scalar(0,0,255)
#define GREEN cv::Scalar(0,255,0)
#define BLUE cv::Scalar(255,0,0)

class Frame {
public:

	Frame();
	Frame(const cv::Mat &frameRaw);
	~Frame();

	void warp(cv::Mat matrix);
	void open();
	void close();
	void setPointValue(const int x, const int y, const uchar val);
	uchar getPointValue(const int x, const int y);
	void cropp(const int xStart, const int yStart, const int height, const int width);
	void histogram(std::vector<int> &histogram);
	void transformToBinary();
	void drawLine(cv::Point &pt1, cv::Point &pt2, const cv::Scalar &color, const int thickness);
	void save(const std::string &filename);
	int getHeight() const;
	int getWidth() const;
	cv::Mat &getRawFrame();

private:

	cv::Mat _frameRaw;
	cv::Mat _kernel;
};	

std::ostream &operator<<(std::ostream &stream, const Frame &frame);
