#pragma once

#include "Events.hpp"

class Observer {
public:

	virtual ~Observer() {};
	virtual void update(Events event) = 0;
};
