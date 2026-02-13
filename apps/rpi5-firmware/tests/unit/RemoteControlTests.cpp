#include "gtest/gtest.h"
#include "RemoteControl.hpp"
#include <cstring>
#include "Mocks.h"

using namespace testing;

class RemoteControlTest : public Test {
protected:
	MockEvdev ev;
	RemoteControl remote{ev};
};

TEST_F(RemoteControlTest, UpdateState) {
	remote.setkey(JoyY, 1);
	EXPECT_EQ(remote.getkey(JoyY), 1);
	remote.setkey(JoyY, 0);
	EXPECT_EQ(remote.getkey(JoyY), 0);
}

TEST_F(RemoteControlTest, EventLoop) {
	struct input_event event;

	std::memset(&event, 0, sizeof(input_event));
	EXPECT_CALL(ev, pendingEvent())
		.WillOnce(Return(1))
		.WillOnce(Return(0));
	EXPECT_CALL(ev, nextEvent())
		.WillOnce(ReturnRef(event));
	remote.getEvent();
}
