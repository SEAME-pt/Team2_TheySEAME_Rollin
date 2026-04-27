#pragma once

#include "Bev.hpp"
#include "Subject.hpp"

#define LEFT -1
#define RIGHT 1

/**
 * @class Lka
 * @brief Lka class
 *
 * The Lka class represents the Lane-Keep-Assist algorithm
 * This class is also responsible to emit events that alter the car steering and throttle
 */
class Lka : public Subject {
public:

	/**
	 * @brief Lka constructor
	 *
	 * Lka constructor.
	 * Defines the Region of Interest of the Frames (Frames will be cropped accordingly)
	 * and the fov for the Birds-Eye transformation
	 *
	 * @param fov for Birds-Eye transformation
	 * @param startX x coord for the top left corner
	 * @param startY y coord for the top left corner
	 * @param width width of the roi rectangle
	 * @param height height of the roi rectangle
	 */
	Lka(const int fov, const int startX, const int startY, const int width, const int height);

	/**
	 * @brief Lka destructor
	 *
	 * Lka destructor.
	 */
	~Lka();

	/**
	 * @brief Get the Lka angle
	 *
	 * Get the current steering angle decided by the Lka algorithm
	 *
	 * @return current lka determined angle
	 */
	int getAngle();

	/**
	 * @brief Lka algorithm
	 *
	 * Lka algorithm.
	 * 1. Transform the Frame to Birds-Eye view
	 * 2. Apply the sliding window algorithm
	 * 3. Calculate the middle lane points
	 * 4. Calculate the Car angle in all middle points and makes an average
	 * 5. Notify the new steering to the Car
	 *
	 * Also draws the lanes from the calculated points and puts it in a screen
	 *
	 * @param frame to apply the lka algo
	 */
	void poly(Frame &frame);

	void putLinesInScreen(Frame &frame, const int ptsNbr, std::vector<cv::Point> &leftLane, std::vector<cv::Point> &rightLane);

private:

	Bev _bev;
	int _angle;
};
