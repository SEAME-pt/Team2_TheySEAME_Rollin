#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include "Frame.hpp"

extern Frame show;

/**
 * @class Bev
 * @brief Bev class
 *
 * The Bev class changes the perspective of a regular Frame into a Birds-Eye-View perspective.
 * This class is mostly used by the Lka class
 */
class Bev {
public:

	/**
	 * @brief Bev constructor
	 *
	 * Bev constructor.
	 * Defines the Region of Interest of the Frames (Frames will be cropped accordingly)
	 * and the fov for the Birds-Eye-View transformation
	 * 
	 * @param fov for Birds-Eye transformation
	 * @param roi Region-Of-Interest rectangle
	 */
	Bev(const int fov, const cv::Rect &roi);

	/**
	 * @brief Bev destructor
	 *
	 * Bev destructor.
	 */
	~Bev();

	/**
	 * @brief Change Frame perspective to Birds-Eye-View
	 *
	 * This algorithm changes the perspective of a regular frame to a Birds-Eye-View perspective
	 * and makes an histogram of it
	 *
	 * Before the perspective transformation, the frame is cropped, using the roi passed in the
	 * constructor, transformed to a binary frame. After the Birds-Eye-View transformation, it is
	 * performed a noise removal in the frame
	 *
	 * @param frame Normal Frame
	 */
	void applyBevToFrame(Frame &frame);

	/**
	 * @brief Get the reverse transformation matrix
	 *
	 * This function returns a matrix capable of reverting the Birds-Eye-View transformation.
	 * This is useful to show the lane markings in the "original" view of the frame
	 *
	 * @return cv::Mat Reverse Matrix
	 */
	cv::Mat &getReverseMatrix();

private:
	
	cv::Rect _roi;
	cv::Mat _M;
	cv::Mat _Mreverse;
	int _fov;
};
