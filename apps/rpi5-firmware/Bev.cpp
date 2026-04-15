#include "Bev.hpp"

Bev::Bev(float imgH, float imgW, void *data) {
	_img = cv::Mat(imgH, imgW, CV_32F, data);
}

Bev::Bev(const cv::Mat &img) {
	_img = img;
}

Bev::~Bev() {}

void Bev::createPerspectiveMatrices(float *srcRaw, float *dstRaw) {
	cv::Mat src(4, 2, CV_32F, srcRaw);
	cv::Mat dst(4, 2, CV_32F, dstRaw);

	_M = cv::getPerspectiveTransform(src, dst);
}

void Bev::warp(cv::Mat *res) {
	cv::warpPerspective(_img, *res, _M, _img.size(), cv::INTER_NEAREST);
}

cv::Mat &Bev::getImg() {
	return (_img);
}
