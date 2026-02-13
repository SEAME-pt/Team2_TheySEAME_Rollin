#include "gtest/gtest.h"
#include "RemoteControl.hpp"
#include "Mocks.h"

class RemoteControlTest : public testing::Test {
protected:
	MockEvdev ev;
	RemoteControl remote{ev};
};

TEST_F(RemoteControlTest, Initial) {
	EXPECT_EQ(remote.getkey(ABS_X), 0);
	EXPECT_EQ(remote.getkey(ABS_Y), 0);
}

/*
 * @brief RemoteControl UpdateState test
 *
 * ====================== Requirement Traceability ===========================
 * ==========================================================================
 */
TEST_F(RemoteControlTest, UpdateState) {
	remote.setkey(KEY_A, 1);
	EXPECT_EQ(remote.getkey(KEY_A), 1);
	remote.setkey(KEY_A, 0);
	EXPECT_EQ(remote.getkey(KEY_A), 0);
}
