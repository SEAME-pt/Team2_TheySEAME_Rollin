#pragma once

#include <opencv4/opencv2/opencv.hpp>
#include "Subject.hpp"
#include <queue>

const int frameW = 640;
const int frameH = 320;

struct Debug {
	int angle;
	float cte;
	int size;
};

/**
 * @class PurePursuit
 * @brief PurePursuit class
 *
 * The PurePursuit class represents the Lane-Keep-Assist algorithm
 * This class is also responsible to emit events that alter the car steering and throttle
 */
class PurePursuit : public Subject {
public:

	/**
	 * @brief PurePursuit constructor
	 *
	 * PurePursuit constructor.
	 * Defines the Region of Interest of the Frames (Frames will be cropped accordingly)
	 * and the fov for the Birds-Eye transformation
	 *
	 * @param fov for Birds-Eye transformation
	 * @param startX x coord for the top left corner
	 * @param startY y coord for the top left corner
	 * @param width width of the roi rectangle
	 * @param height height of the roi rectangle
	 */
	PurePursuit();

	/**
	 * @brief PurePursuit destructor
	 *
	 * PurePursuit destructor.
	 */
	~PurePursuit();

	/**
	 * @brief Get the PurePursuit angle
	 *
	 * Get the current steering angle decided by the PurePursuit algorithm
	 *
	 * @return current lka determined angle
	 */
	int getAngle();

	/**
	 * @brief PurePursuit algorithm
	 *
	 * PurePursuit algorithm.
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
	 struct Debug control(float leftK, float rightK, float x1, float x2);

	void debug(float leftK, float rightK, float fbangle, float ffangle, float angle, int diff, float cteOff);
	void validateCurves(float leftK, float rightK);

	float calcCurve(float leftK, float rightK);

private:

	float calcAngle(float k);

	const float _alpha;
	const float _L;
	const float _kCte;
	const int _stallFrames;
	const int _angleToll;
	const int _offset;
	const int _kTreshold;
	const float _kStep;
	const float _kAlpha;
	const float _cteAlpha;
	float _prevCteNorm;
	bool _badleftK;
	bool _badrightK;
	int _badleftKI;
	int _badrightKI;
	int _lastlw;
	float _prevleftK;
	float _prevrightK;
	float _prevK;
	float _prevAngle;
	int _showAngle;
	std::queue<int> _angle;
};

