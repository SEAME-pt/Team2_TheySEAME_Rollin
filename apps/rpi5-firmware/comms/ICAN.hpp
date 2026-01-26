#pragma once

#include <linux/can.h>
#include <stdint.h>
#include <string>

class ICAN {
public:

	virtual ~ICAN() {};

	/**
	 * @brief Opens a CAN socket
	 *
	 * Puts the CAN interface UP, creates a CAN socket and binds it
	 *
	 * @return 0 if success
	 * @return -1 if failed
	 *
	 */
	virtual int openSocket() = 0;

	/**
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
	virtual int readFrame(struct can_frame &frame) = 0;

	/**
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
	virtual int sendFrame(const canid_t id, const uint8_t *data, const uint8_t len) = 0;

	/**
	 * @brief Get the CAN socket fd
	 *
	 * Returns the CAN socket fd
	 *
	 * @return CAN socket fd
	 *
	 */
	virtual int getSocketFd() const = 0;

	/**
	 * @brief Get the CAN interface name
	 *
	 * Returns the CAN interface name. e.g: "can0"
	 *
	 * @return CAN interface name
	 *
	 */
	virtual std::string getInterface() const = 0;

	/**
	 * @brief Gets CAN bitrate
	 *
	 * Returns the current CAN bitrate
	 * It makes a netlink request to get the current bitrate
	 *
	 * @return bitrate
	 *
	 */
	virtual unsigned int getBitrate() const = 0;

	/**
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
	virtual int setBitrate(unsigned int bitrate) = 0;

	/**
	 * @brief Get the CAN mode
	 *
	 * Returns the current turned on mode in CAN
	 * This is the current active mode
	 *
	 * @return CAN active mode
	 *
	 */
	virtual unsigned int getActiveMode() const = 0;

	/**
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
	virtual int setMode(unsigned int modeToControl, unsigned int modeToTurnOn) = 0;

	/**
	 * @brief Is CAN interface UP
	 *
	 * Returns the current CAN bitrate
	 * It makes a netlink request to get the current bitrate
	 *
	 * @return true if CAN is UP
	 * @return false if CAN is DOWN
	 *
	 */
	virtual bool isUp() const = 0;
};
