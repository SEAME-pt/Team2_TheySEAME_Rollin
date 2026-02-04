#include "Subject.hpp"
#include <iostream>

Subject::Subject() {
	std::cout << "Subject constructor" << std::endl;
	_obs = NULL;
}

Subject::~Subject() {
	std::cout << "Subject destructor" << std::endl;
}

void Subject::attach(Observer *o) {
	_obs = o;
}

void Subject::detach(Observer *o) {
	_obs = NULL;
}

void Subject::notify(Events event) {
	if (_obs == NULL) {
		return;
	}
	_obs->update(event);
}
