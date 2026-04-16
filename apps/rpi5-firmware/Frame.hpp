#pragma once

#include <opencv4/opencv2/core.hpp>

class Frame {
public:

	Frame();
	~Frame();

	void warp(cv::Mat matrix);
	void open();
	void close();
	void cropp();
	void transformToBinary();
	int getHeight();
	int getWidth();

private:

	cv::Mat _frameRaw;
	cv::Mat _kernel;
};	
