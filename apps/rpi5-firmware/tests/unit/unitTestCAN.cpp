#include "gtest/gtest.h"
#include "CAN.hpp"

namespace Test {
	#include "FakeSocketCan.cpp"
}

TEST(CAN, unit_test) {
	CAN can("can0");
	EXPECT_EQ(1, 1);
}
