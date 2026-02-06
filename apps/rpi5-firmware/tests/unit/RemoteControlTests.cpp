#include "gtest/gtest.h"
#include "RemoteControl.hpp"
#include "Mocks.h"

class RemoteControlTest : public testing::Test {
protected:
	MockEvdev ev;
	RemoteControl remote{ev};
};

/*
 * @brief RemoteControl UpdateState test
 *
 * ====================== Requirement Traceability ===========================
 * [test->dsn~remote-control-read-inputs~1]
 * ==========================================================================
 */
TEST_F(RemoteControlTest, UpdateState) {
	remote.setkey(KEY_A, 1);
	EXPECT_EQ(remote.getkey(KEY_A), 1);
	remote.setkey(KEY_A, 0);
	EXPECT_EQ(remote.getkey(KEY_A), 0);
}
