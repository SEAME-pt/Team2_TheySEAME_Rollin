#pragma once

#include <opencv4/opencv2/opencv.hpp>

class Bev {
public:

	Bev(float imgH, float imgW, void *data, const int fov);
	Bev(const cv::Mat &img);
	~Bev();

	void createPerspectiveMatrices(float *srcRaw, float *dstRaw);
	void warp(cv::Mat *res);
	cv::Mat &getImg();

private:
	
	cv::Mat _img;
	cv::Mat _M;
	int _fov;
};
