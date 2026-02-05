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
	static const int size = 10;
	int _obsNbr;
	Observer *_obs[size];
};
