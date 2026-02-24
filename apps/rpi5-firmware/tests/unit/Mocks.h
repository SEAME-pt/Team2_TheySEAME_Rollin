#pragma once

#include "gmock/gmock.h"
#include "../mode_manager/remote/IEvdev.hpp"
#include "Observer.hpp"
#include "ICAN.hpp"

class MockEvdev : public IEvdev {
public:
	MOCK_METHOD(int, getfd, (), (const, override));
	MOCK_METHOD(const char*, getDevice, (), (const, override));
	MOCK_METHOD(void, readEvent, (), (override));
	MOCK_METHOD(struct input_event&, nextEvent, (), (override));
	MOCK_METHOD(int, pendingEvent, (), (const, override));
};

class MockObserver : public Observer {
public:
	MOCK_METHOD(void, update, (Events event), (override));
};

class MockCan : public ICAN {
public:
	MOCK_METHOD(int, openSocket, (), (override));
	MOCK_METHOD(int, readFrame, (struct can_frame &), (override));
	MOCK_METHOD(int, sendFrame, (const canid_t, const uint8_t *data, const uint8_t len), (override));
	MOCK_METHOD(int, getSocketFd, (), (const, override));
	MOCK_METHOD(std::string, getInterface, (), (const, override));
	MOCK_METHOD(unsigned int, getBitrate, (), (const, override));
	MOCK_METHOD(int, setBitrate, (unsigned int), (override));
	MOCK_METHOD(unsigned int, getActiveMode, (), (const, override));
	MOCK_METHOD(int, setMode, (unsigned int, unsigned int), (override));
	MOCK_METHOD(bool, isUp, (), (const, override));
};
