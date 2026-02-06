#pragma once

#include "Events.hpp"

/**
 * @class Observer
 * @brief Observer interface
 *
 * The Observer interface defines the contract to receive events from a Subject
 */
class Observer {
public:

	virtual ~Observer() {};

	/**
	 * @brief Update after a notify
	 *
	 * Updates Observer accordingly the Event sent.
	 * This function is called whenver a Subject calls notify
	 *
	 * @param event Describe the Event that triggered the notify
	 */
	virtual void update(Events event) = 0;
};
