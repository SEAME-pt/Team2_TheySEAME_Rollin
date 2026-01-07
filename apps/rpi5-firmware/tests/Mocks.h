#pragma once

#include <gmock/gmock.h>
#include "../mode_manager/remote/IEvdev.hpp"

class MockEvdev : public IEvdev {
public:
	MOCK_METHOD(int, getfd, (), (const,override));
	MOCK_METHOD(const char*, getDevice, (), (const,override));
	MOCK_METHOD(void, readEvent, (), (override));
	MOCK_METHOD(struct input_event&, nextEvent, (), (override));
	MOCK_METHOD(int, pendingEvent, (), (const, override));
};
