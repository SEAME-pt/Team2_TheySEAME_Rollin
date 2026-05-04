#pragma once

#include <opencv4/opencv2/core.hpp>

#define RED cv::Scalar(0,0,255)
#define GREEN cv::Scalar(0,255,0)
#define BLUE cv::Scalar(255,0,0)
#define YELLOW cv::Scalar(0, 255, 255)
#define WHITE cv::Scalar(255, 255, 255)

#define WHITE_PIXEL 1

// This is all 1 channel Types
#define BINARY_FRAME CV_8U
#define INT_FRAME CV_32S
#define FLOAT_FRAME CV_32F

/**
 * @class Frame
 * @brief Frame class
 *
 * The Frame class represents an opencv frame coming from a video or image.
 * This is mostly a wrapper of the cv::Mat object applied to video frames
 */
class Frame {
public:

	/**
	 * @brief Default Frame constructor
	 *
	 * Defaults Frame constructor
	 *
	 * @param cv::Mat frame
	 */
	Frame();

	/**
	 * @brief Frame constructor
	 *
	 * Wrapps the frame parameter
	 *
	 * @param cv::Mat frame
	 */
	Frame(const cv::Mat &frameRaw);

	/**
	 * @brief Frame constructor
	 *
	 * Creates a cv::Mat object with the height and width and type passed.
	 * For now, a frame can be of type:
	 * - BINARY
	 * - INT
	 * - FLOAT
	 * All of these types are 1 channel
	 *
	 * @param cv::Mat frame
	 */
	Frame(const int height, const int width, const int type);

	/*
	 * @brief Frame destructor
	 *
	 * Frame destructor
	 */
	~Frame();

	/**
	 * @brief Frame Overload = operator
	 *
	 * Copies the frameRaw and returns the new Frame
	 *
	 * @param frame that will be copied
	 *
	 * @return New Frame equal to the passed one
	 */
	Frame &operator=(const Frame &frame);

	/**
	 * @brief Warp a Frame
	 *
	 * Applies a transformation matrix to the frame distorting it accordingly
	 *
	 * @param transformation matrix
	 */
	void warp(cv::Mat matrix);

	/**
	 * @brief Open a Frame
	 *
	 * Applies an Open morphological transformation to the image
	 * An Open morph trasnformation erodes then dilates the frame
	 */
	void open();

	/**
	 * @brief Closes a Frame
	 *
	 * Applies an Close morphological transformation to the image
	 * An Close morph trasnformation dilates then erodes the frame
	 */
	void close();

	/**
	 * @brief Canny a Frame
	 *
	 * Detect edges in a binary frame, and substitutes the current frame
	 * with the edges frame
	 */
	void canny();

	/**
	 * @brief Change a Pixel value
	 *
	 * Change the value of a Pixel in a Frame. Only works for binary frames
	 *
	 * @param x coord
	 * @param y coord
	 * @param val pixel value
	 */
	void setPointValue(const int x, const int y, const uchar val);

	/**
	 * @brief Get a Pixel value
	 *
	 * Get the value of a Pixel in a Frame. Only works for binary frames
	 *
	 * @param x coord
	 * @param y coord
	 */
	uchar getPointValue(const int x, const int y);

	/**
	 * @brief Cropp a Frame
	 *
	 * Cropp the Frame accordingly the Region of Interest given.
	 * The cropped Frame will substitute the old Frame
	 *
	 * @param rect Region of Interest
	 */
	void cropp(const cv::Rect &rect);

	/**
	 * @brief Create an histogram
	 *
	 * Create an histogram of the Frame.
	 * This sums all the columns of the Frame and turns into a 1D vector/array
	 *
	 * @param histogram vector to store the histogram
	 */
	void histogram(std::vector<int> &histogram);

	/**
	 * @brief Transform a Frame to Binary
	 *
	 * Change the current Frame to be a binary frame.
	 * Higher thresh value = less white pixels in the frame
	 *
	 * @param thresh Threshold value
	 */
	void transformToBinary(const int thresh);

	/**
	 * @brief Create a copy Colored Frame
	 *
	 * Creates a new Frame equal to the current but in Color mode
	 *
	 * @return Frame in color mode
	 */
	Frame getColoredFrame();

	/**
	 * @brief Draw a line in a Frame
	 *
	 * Draws a line between 2 points in a Frame with a variable color and thickness.
	 * The colors will only be seen if the frame is in Color mode
	 *
	 * @param pt1 Point 1
	 * @param pt2 Point 2
	 * @param Scalar cv::Scalar object in BGR mode
	 * @param thickness Line thickness. Higher the value, higher the thickness
	 */
	void drawLine(cv::Point &pt1, cv::Point &pt2, const cv::Scalar &color, const int thickness);

	/**
	 * @brief Save a Frame as an Image
	 *
	 * Save the current Frame as an Image
	 *
	 * @param filename Image name
	 */
	void save(const std::string &filename);

	/**
	 * @brief Put the Frame in a Screen
	 *
	 * Put the current Frame in a Screen in a grpahical enviroment
	 *
	 * @param winName opencv Window name
	 */
	void showInScreen(const std::string &winName);

	/**
	 * @brief Get Frame height
	 *
	 * Get the Frame height
	 *
	 * @return the current height
	 */
	int getHeight() const;

	/**
	 * @brief Get Frame width
	 *
	 * Get the Frame width
	 *
	 * @return the current width
	 */
	int getWidth() const;

	/**
	 * @brief Get Frame raw data
	 *
	 * Get access to the raw matrix of a frame
	 *
	 * @return void pointer to the data
	 */
	void *getRawData();

	/**
	 * @brief Get Frame Mat Object
	 *
	 * Get access to the wrapped cv::Mat object
	 *
	 * @return cv::Mat Object
	 */
	cv::Mat &getMatObj();

private:

	cv::Mat _frameRaw;
	cv::Mat _kernel;
};	

std::ostream &operator<<(std::ostream &stream, const Frame &frame);
