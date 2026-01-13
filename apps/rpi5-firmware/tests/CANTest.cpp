#include "gtest/gtest.h"
#include "../comms/CAN.hpp"

class CANTest : public testing::Test {
protected:

	CANTest() {}
	~CANTest() {}

	void SetUp() override {
		_can = new CAN("can0");
		_can->setBitrate(500);
		_can->setMode(CAN_CTRLMODE_LOOPBACK, CAN_CTRLMODE_LOOPBACK);
	}

	void TearDown() override {
		delete _can;
	}

	ICAN *_can;
};


/*
 * @brief Verify the CAN Interface in agreement to the requirements
 *
 * ====================== Requirement Traceability ===========================
 * [test->dsn~comms-can-rpi-interface~1]
 * ==========================================================================
 */
TEST_F(CANTest, CANInterface) {
	bool isUp = _can->isUp();
	ASSERT_EQ(isUp, false);
	unsigned int bitrate = _can->getBitrate();
	ASSERT_EQ(bitrate, 500);
	unsigned int mode = _can->getActiveMode();
	ASSERT_EQ(mode, CAN_CTRLMODE_LOOPBACK);
}

/*
 * @brief Test CAN in loopback mode
 *
 * ====================== Requirement Traceability ===========================
 * [test->dsn~comms-can-rpi-receiveMsg~2]
 * [test->dsn~comms-can-rpi-sendMsg~1]
 * ==========================================================================
 */
TEST_F(CANTest, SendAndReadFrame) {
	struct can_frame frame;
	int ret;
	canid_t id = 0x100;
	uint8_t data[8] = { 0x10, 0x30, 0, 0, 0, 0, 0, 0 };

	_can->openSocket();
	ASSERT_EQ(_can->isUp(), true);
	ret = _can->sendFrame(id, data, sizeof(data));
	ASSERT_EQ(0, ret);

	ret = _can->readFrame(frame);
	ASSERT_EQ(0, ret);
	EXPECT_EQ(id, frame.can_id);
	for (int i = 0; i < sizeof(frame.data); i++) {
		EXPECT_EQ(data[i], frame.data[i]);
	}
	EXPECT_EQ(sizeof(data), frame.len);
}
