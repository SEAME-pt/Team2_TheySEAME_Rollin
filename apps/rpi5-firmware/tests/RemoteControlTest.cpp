#include "gtest/gtest.h"
#include "../mode_manager/remote/RemoteControl.hpp"
#include "Mocks.h"

/*
 * @brief RemoteControl UpdateState test
 *
 * ====================== Requirement Traceability ===========================
 * [test->dsn~remote-control-read-inputs~1]
 * ==========================================================================
 */
TEST(RemoteControl, UpdateState) {
	MockEvdev ev;
	RemoteControl remote(ev);

	remote.setkey(KEY_A, 1);
	EXPECT_EQ(remote.getkey(KEY_A), 1);
	remote.setkey(KEY_A, 0);
	EXPECT_EQ(remote.getkey(KEY_A), 0);
}
