#pragma once

#include "Frame.hpp"

/**
 * @class SlidingWindow
 * @brief SlidingWindow class
 *
 * The SlidingWindow class represents the slidingWindow algorithm to get lane segments
 * For now it returns the left and right lane points
 */
class SlidingWindow {
public:

	/**
	 * @brief SlidingWindow constructor
	 *
	 * SlidingWindow constructor
	 */
	SlidingWindow();

	/**
	 * @brief SlidingWindow destructor
	 *
	 * SlidingWindow destructor
	 */
	~SlidingWindow();

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
	 * @param ptnsNbr Number of Windows to apply
	 * @param rectW Width of the Windows
	 * @param ptns Vector to store the average Points found
	 */
	void slidingWindow(Frame &frame, int startX, int ptnsNbr, int rectW, std::vector<cv::Point> &ptns);

	/**
	 * @brief Returns the left and right lane points accordingly
	 *
	 * This function wrapps the sliding window algorithm and returns the left and right
	 * lane points accordingly. For more information about the sliding window algorithm
	 * check it's own function
	 *
	 * @param frame Current frame
	 * @param nbrPtns Number of points to return for each lane
	 * @param leftLanePtns Vector holding all the leftLane points
	 * @param rightLanePtns Vector holding all the rightLane points
	 */
	void getLanePtns(Frame &frame, const int nbrPtns, std::vector<cv::Point> &leftLanePtns, std::vector<cv::Point> &rightLanePtns);

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
	 * @param histogram Current frame Histogram
	 *
	 * @return int X coordinate of a lane marking
	 */
	int getLaneX(std::vector<int> &histogram, std::vector<int>::iterator hstart, std::vector<int>::iterator hend);

private:

	/**
	 * @brief Check white pixels in the rectangle region
	 *
	 * Check all the pixels in a region and computes the average x coordinate of all white pixels
	 *
	 * @return int Average of all white pixels x coordinates
	 */
	int checkPixelsInRect(Frame &frame, cv::Rect &rect);
};
