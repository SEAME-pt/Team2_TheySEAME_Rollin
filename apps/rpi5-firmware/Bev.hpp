#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include "Frame.hpp"

class Bev {
public:

	Bev(const int fov, const int frame_h, const int frame_w);
	~Bev();

	cv::Mat &createPerspectiveMatrices(float *srcRaw, float *dstRaw);
	void slidingWindow(Frame &frame, int x, int ptnNbr, int rectW, std::vector<cv::Point> &ptns);
	void applyBevToFrame(Frame &frame);

private:
	
	int checkPixelsInRect(Frame &frame, cv::Rect &rect);

	cv::Mat _M;
	int _fov;
};
