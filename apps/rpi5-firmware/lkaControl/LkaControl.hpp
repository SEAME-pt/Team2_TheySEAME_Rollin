#pragma once

#include "Subject.hpp"
#include <queue>

const int frameW = 640;
const int frameH = 320;

struct Debug {
	int angle;
	float cte;
	float heading;
};

/**
 * @class LkaControl
 * @brief LkaControl class
 *
 * The LkaControl class represents the Lane-Keep-Assist algorithm
 * This class is also responsible to emit events that alter the car steering and throttle
 */
class LkaControl : public Subject {
public:

	/**
	 * @brief LkaControl constructor
	 *
	 * LkaControl constructor.
	 * Defines the Region of Interest of the Frames (Frames will be cropped accordingly)
	 * and the fov for the Birds-Eye transformation
	 *
	 * @param fov for Birds-Eye transformation
	 * @param startX x coord for the top left corner
	 * @param startY y coord for the top left corner
	 * @param width width of the roi rectangle
	 * @param height height of the roi rectangle
	 */
	LkaControl();

	/**
	 * @brief LkaControl destructor
	 *
	 * LkaControl destructor.
	 */
	~LkaControl();

	/**
	 * @brief Get the LkaControl angle
	 *
	 * Get the current steering angle decided by the LkaControl algorithm
	 *
	 * @return current lka determined angle
	 */
	int getAngle();

	/**
	 * @brief LkaControl algorithm
	 *
	 * LkaControl algorithm.
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
	 struct Debug control(float x1, float x2, float lftA1, float lftB1, float rghtA1, float rghtB1);

	void debug(float angle, float diff, float cteOff, int lw, float heading);

private:

	const float _kCte;
	const int _stallFrames;
	const int _angleToll;
	const int _offset;
	const float _cteAlpha;
	const float _camOffset;
	const int _frameTime;
	float _prevCteNorm;
	int _lastlw;
	float _prevAngle;
	int _showAngle;
	int _timeLapse;
	std::queue<int> _angle;
};

