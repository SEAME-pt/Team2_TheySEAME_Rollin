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
	 * Init all control and constant variables for the LKA
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
	 * Apply the Stanley Algorithm to calculate the steering angle and apply it to the car
	 *
	 * @param x1 X coordinate of a point in the left lane
	 * @param x2 X coordinate of a point in the right lane
	 * @param lftA1 Left lane parametric coefficients for the X value
	 * @param lftB1 Left lane parametric coefficients for the Y value
	 * @param rghtA1 Right lane parametric coefficients for the X value
	 * @param rghtB1 Right lane parametric coefficients for the Y value
	 *
	 * @return Debug values to print to the screen in the python script
	 */
	struct Debug control(float x1, float x2, float lftA1, float lftB1, float rghtA1, float rghtB1);
	
	/**
	 * @brief Debug Received Values
	 *
	 * Debug control values
	 * Print the most important values for the LKA to work
	 *
	 * @param angle Recent calculated angle
	 * @param diff Difference between the current and last calculated angle
	 * @param cteOff Calculated CTE offset
	 * @param lw Calculated Lane width
	 * @param heading Current car heading angle
	 */
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

