#include "CAN.hpp"
#include <linux/rtnetlink.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <cstdio>
#include <cstring>
#include <iostream>

/*
 * @brief CAN constructor
 *
 * @param interface name of the CAN interface
 *
 * Accepts the CAN interface name to use and stops the CAN interface.
 * This opens the possibility to configure the CAN interface
 * outside the constructor
 *
 */
CAN::CAN(const std::string &interface) : _interface(interface) {
	_up = false;
	can_do_stop(_interface.c_str());
}

/*
 * @brief CAN constructor
 *
 * @param interface name of the CAN interface
 * @param bitrate bitrate to set the CAN interface
 * @param modeToControl modes to control in the CAN
 * @param modeToTurnOn mode from modesToControl to be active
 *
 * Constructs with the given configuration. This configuration is done
 * inside the constructor and the CAN socket is open after it
 * More about modes in setMode() function
 *
 */
CAN::CAN(const std::string &interface, const unsigned int bitrate, 
	unsigned int modeToControl, unsigned int modeToTurnOn) : _interface(interface) {
	_up = false;
	can_do_stop(_interface.c_str());
	setBitrate(bitrate);
	setMode(modeToControl, modeToTurnOn);
	openSocket();
}

/*
 * @brief CAN destructor
 *
 * This CAN destructor closes the CAN socket fd and stops the CAN interface
 *
 */
CAN::~CAN() {
	close(_sock);
	can_do_stop(_interface.c_str());
	std::cout << "Closed CAN socket" << std::endl;
}

/*
 * @brief Set CAN bitrate
 *
 * @param bitrate bitrate to set in kbit/s
 *
 * This function sets the given bitrate to the CAN interface
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-interface~1]
 * ==========================================================================
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
int CAN::setBitrate(unsigned int bitrate) {
	return (can_set_bitrate(_interface.c_str(), bitrate * 1000));
}

/*
 * @brief Set CAN control mode
 *
 * @param modeToControl Selected modes which you want to control
 * @param modeToTurnOn Mode to from the selected ones to turn on
 *
 * This functions accepts the modes in which the CAN controller can control (modesToControl)
 * and the mode to set as active from the modesToControl (modeToTurnOn). The other selected modes
 * different than modeToTurnOn will be turned off
 *
 * @code
 * #define CAN_CTRLMODE_LOOPBACK           0x01    // Loopback mode
 * #define CAN_CTRLMODE_LISTENONLY         0x02    // Listen-only mode
 * #define CAN_CTRLMODE_3_SAMPLES          0x04    // Triple sampling mode
 * #define CAN_CTRLMODE_ONE_SHOT           0x08    // One-Shot mode
 * #define CAN_CTRLMODE_BERR_REPORTING     0x10    // Bus-error reporting
 * #define CAN_CTRLMODE_FD                 0x20    // CAN FD mode
 * #define CAN_CTRLMODE_PRESUME_ACK        0x40    // Ignore missing CAN ACKs
 * @endcode
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
int CAN::setMode(unsigned int modeToControl, unsigned int modeToTurnOn) {
	struct can_ctrlmode canMode;

	canMode.mask = modeToControl;
	canMode.flags = modeToControl;
	return (can_set_ctrlmode(_interface.c_str(), &canMode));
}

/*
 * @brief Get the CAN mode
 *
 * Returns the current turned on mode in CAN
 * This is the current active mode
 *
 * @return CAN active mode
 *
 */
unsigned int CAN::getActiveMode() const {
	struct can_ctrlmode cm;

	can_get_ctrlmode(_interface.c_str(), &cm);
	printf("CMode %u\n", cm.mask);
	return (cm.flags);
}

/*
 * @brief Get the CAN socket fd
 *
 * Returns the CAN socket fd
 *
 * @return CAN socket fd
 *
 */
int CAN::getSocketFd() const { return (_sock); }

/*
 * @brief Get the CAN interface name
 *
 * Returns the CAN interface name. e.g: "can0"
 *
 * @return CAN interface name
 *
 */
std::string CAN::getInterface() const { return (_interface); }

/*
 * @brief Is CAN interface UP
 *
 * Returns the current CAN bitrate
 * It makes a netlink request to get the current bitrate
 *
 * @return true if CAN is UP
 * @return false if CAN is DOWN
 *
 */
bool CAN::isUp() const { return (_up); }

/*
 * @brief Gets CAN bitrate
 *
 * Returns the current CAN bitrate
 * It makes a netlink request to get the current bitrate
 *
 * @return bitrate
 *
 */
unsigned int CAN::getBitrate() const {
	struct can_bittiming bt;

	can_get_bittiming(_interface.c_str(), &bt);
	return (bt.bitrate / 1000);
};

/*
 * @brief Sends a CAN frame to the Bus
 *
 * @param id frame CAN id
 * @param data data to send (0-8 bytes)
 * @param len length of the data (0-8 bytes)
 *
 * This function fills the can_frame struct with the id, length and data and
 * sends the frame to the Bus
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-sendMsg~1]
 * ==========================================================================
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
int CAN::sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) {
	struct can_frame frame;
	int nbytes;

	frame.can_id = id;
	frame.len = len;
	std::memcpy(frame.data, data, frame.len);

	nbytes = write(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in write");
		return (-1);
	}
	return (0);
}

/*
 * @brief Read a CAN frame from the Bus
 *
 * @param frame reference to a frame struct
 *
 * This function reads a CAN frame from the Bus and puts it into the
 * the can_frame reference
 *
 * ====================== Requirement Traceability ===========================
 * [impl->dsn~comms-can-rpi-receiveMsg~2]
 * ==========================================================================
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
int CAN::readFrame(struct can_frame &frame) {
	int nbytes;

	nbytes = read(_sock, &frame, sizeof(struct can_frame));
	if (nbytes < 0) {
		std::perror("Error in read");
		return (-1);
	}
	return (0);
}

/*
 * @brief Opens a CAN socket
 *
 * Puts the CAN interface UP, creates a CAN socket and binds it
 *
 * @return 0 if success
 * @return -1 if failed
 *
 */
int CAN::openSocket() {
	struct sockaddr_can addr;
	struct ifreq ifr;

	if (can_do_start(_interface.c_str()) < 0) {
		return (-1);
	}
	_up = true;
	_sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
	if (_sock < 0) {
		return (-1);
	}
	std::cout << "Created Socket" << std::endl;

	std::strcpy(ifr.ifr_name, _interface.c_str());
	if (ioctl(_sock, SIOCGIFINDEX, &ifr) < 0) {
		return (-1);
	}
	addr.can_family = AF_CAN;
	addr.can_ifindex = ifr.ifr_ifindex;
	if (bind(_sock, (struct sockaddr *)&addr, sizeof(struct sockaddr_can)) < 0) {
		return (-1);
	}
	std::cout << "Binded Socket" << std::endl;
	return (0);
}

