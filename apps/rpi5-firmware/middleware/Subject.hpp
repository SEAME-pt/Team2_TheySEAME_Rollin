#pragma once

#include "Observer.hpp"

class Subject {
public:

	Subject();
	~Subject();

	void attach(Observer *o);
	void detach(Observer *o);
	void notify(Events event);

private:
	Observer *_obs;
};
