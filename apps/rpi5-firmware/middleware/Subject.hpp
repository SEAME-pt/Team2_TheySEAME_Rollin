#pragma once

#include "Observer.hpp"

class Observer;

/**
 * @class Subject
 * @brief Subject abstract class
 *
 * The Subject is the Event emitter to the Observers.
 * Any class can inherit the Subject class if it wants to emitt events
 * to other classes (Observers)
 */
class Subject {
public:

	/**
	 * @brief Subject Constructor
	 *
	 * Construct a Subject with space for 10 Observers
	 */
	Subject();

	/**
	 * @brief Subject Destructor
	 *
	 * Subject Destructor function
	 */
	~Subject();

	/**
	 * @brief Attach an Observer
	 *
	 * Add an Observer to the Subject, so whenever a notify call is made,
	 * the Observer is notified
	 *
	 * @param o Observer class
	 */
	void attach(Observer *o);

	/**
	 * @brief Detach an Observer
	 *
	 * Remove an Observer in the Subject, so whenever a notify call is made,
	 * the Observer isn't notified
	 *
	 * @param o Observer class
	 */
	void detach(Observer *o);

	/**
	 * @brief Notify Observers
	 *
	 * Subject notifies it's Observers of a specific event.
	 * This is done through the Observer.update(event) method
	 *
	 * @param event Describe the Event that triggered the notify
	 */
	void notify(Events event);

	static const int size = 10;

private:
	int _obsNbr;
	Observer *_obs[size];
};
