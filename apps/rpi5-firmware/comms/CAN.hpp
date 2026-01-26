#pragma once

#include "ICAN.hpp"
#include <net/if.h>
#include <libsocketcan.h>

class CAN : public ICAN {
public:

	/**
	 * @brief CAN constructor
	 *
	 * Accepts the CAN interface name to use and stops the CAN interface.
	 * This opens the possibility to configure the CAN interface
	 * outside the constructor
	 *
	 * @param interface name of the CAN interface
	 *
	 */
	CAN(const std::string &interface);

	/**
	 * @brief CAN constructor
	 *
	 * Constructs with the given configuration. This configuration is done
	 * inside the constructor and the CAN socket is open after it
	 * More about modes in setMode() function
	 *
	 * @param interface name of the CAN interface
	 * @param bitrate bitrate to set the CAN interface
	 * @param modeToControl modes to control in the CAN
	 * @param modeToTurnOn mode from modesToControl to be active
	 *
	 */
	CAN(const std::string &interface, const unsigned int bitrate,
		unsigned int modeToControl, unsigned int modeToTurnOn);

	/*
	 * @brief CAN destructor
	 *
	 * This CAN destructor closes the CAN socket fd and stops the CAN interface
	 *
	 */
	~CAN();

	int openSocket();
	int readFrame(struct can_frame &frame);
	int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len);
	int getSocketFd() const;
	std::string getInterface() const;
	unsigned int getBitrate() const;
	int setBitrate(unsigned int bitrate);
	unsigned int getActiveMode() const;
	int setMode(unsigned int modeToControl, unsigned int modeToTurnOn);
	bool isUp() const;

private:
	const std::string _interface;
	bool _up;
	int _sock;
};
