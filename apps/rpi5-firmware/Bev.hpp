#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include "Frame.hpp"

/**
 * @class Bev
 * @brief Bev class
 *
 * The Bev class changes the perspective of a regular Frame into a Birds-Eye-View perspective.
 * It also implements the Sliding Window algorithm.
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
	 * @brief Detects lane points in a binary image using the Sliding Window algorithm
	 *
	 * This algorithm scans a binary frame from top to bottom using reactangle windows
	 * in search of lane markings. Each window is centered around the current x coor and searchs
	 * for white pixels (lane marking) within it's region
	 *
	 * Each window:
	 * - Gets all the white pixels in it's region
	 * - Computes the average x coordinate of this white pixels
	 * - Stores this average (x, y) coordinate in the vector
	 * - The next window is centerd around the average x of the last window
	 *
	 * If a window doesn't find white pixels, the next window will have the same x coordinate
	 *
	 * This function can onlt be called after the Bev transformation function
	 * 
	 * @param frame Binary Frame where lane markings are represented as white pixels
	 * @param x Initial coordinate for the first Window
	 * @param ptnNbr Number of Windows to apply
	 * @param rectW Width of the Windows
	 * @param ptns Vector to store the average Points found
	 */
	void slidingWindow(Frame &frame, int x, int ptnNbr, int rectW, std::vector<cv::Point> &ptns);

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
	 * This function if for frames that don't pass to the AI model
	 *
	 * @param frame Normal Frame
	 */
	void applyBevToFrameTD(Frame &frame);

	/**
	 * @brief Change Frame perspective to Birds-Eye-View
	 *
	 * This algorithm changes the perspective of a regular frame to a Birds-Eye-View perspective
	 * and makes an histogram of it
	 *
	 * This function is for frames that come from the AI model
	 *
	 * @param frame Normal Frame
	 */
	void applyBevToFrameAI(Frame &frame);

	/**
	 * @brief Get the reverse transformation matrix
	 *
	 * This function returns a matrix capable of reverting the Birds-Eye-View transformation.
	 * This is useful to show the lane markings in the "original" view of the frame
	 *
	 * @return cv::Mat Reverse Matrix
	 */
	cv::Mat &getReverseMatrix();

	/**
	 * @brief Get a lane x coor
	 *
	 * This function returns the most probable x coordinate of a lane marking and
	 * clears the markings around. This is done to ensure the next call returns the other lane.
	 *
	 * Under the hood, this functions just finds the highest number in an histogram,
	 * clears the surronding numbers and returns it's index. The index represents the x coordinate 
	 * of the lane
	 *
	 * @return int X coordinate of a lane marking
	 */
	int getLaneX();

private:
	
	/**
	 * @brief Check white pixels in the rectangle region
	 *
	 * Check all the pixels in a region and computes the average x coordinate of all white pixels
	 *
	 * @return int Average of all white pixels x coordinates
	 */
	int checkPixelsInRect(Frame &frame, cv::Rect &rect);

	cv::Rect _roi;
	cv::Mat _M;
	cv::Mat _Mreverse;
	std::vector<int> _histogram;
	int _fov;
};
