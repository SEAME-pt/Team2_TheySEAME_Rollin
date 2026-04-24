#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include "Frame.hpp"

class Bev {
public:

	Bev(const int fov, const cv::Rect &roi);
	~Bev();

	cv::Mat &createPerspectiveMatrices(float *srcRaw, float *dstRaw);
	void slidingWindow(Frame &frame, int x, int ptnNbr, int rectW, std::vector<cv::Point> &ptns);
	void applyBevToFrameTD(Frame &frame);
	void applyBevToFrameAI(Frame &frame);
	cv::Mat &getReverseMatrix();
	int getLaneX();

private:
	
	int checkPixelsInRect(Frame &frame, cv::Rect &rect);

	cv::Rect _roi;
	cv::Mat _M;
	cv::Mat _Mreverse;
	std::vector<int> _histogram;
	int _fov;
};
