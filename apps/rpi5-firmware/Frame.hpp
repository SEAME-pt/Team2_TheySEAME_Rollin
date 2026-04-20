#pragma once

#include <opencv4/opencv2/core.hpp>

class Frame {
public:

	Frame();
	Frame(const cv::Mat &frameRaw);
	~Frame();

	void warp(cv::Mat matrix);
	void open();
	void close();
	void cropp();
	void transformToBinary();
	void save(const std::string &filename);
	int getHeight() const;
	int getWidth() const;
	cv::Mat &getRawFrame();

private:

	cv::Mat _frameRaw;
	cv::Mat _kernel;
};	

std::ostream &operator<<(std::ostream &stream, const Frame &frame);
