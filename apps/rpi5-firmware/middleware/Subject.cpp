#include "Subject.hpp"
#include <iostream>
#include <cstring>

Subject::Subject() {
	std::cout << "Subject constructor" << std::endl;
	std::memset(_obs, 0, size * sizeof(Observer *));
	_obsNbr = 0;
}

Subject::~Subject() {
	std::cout << "Subject destructor" << std::endl;
}

void Subject::attach(Observer *o) {
	if (_obsNbr >= size) {
		std::cout << "Observer Array is full" << std::endl;
		return;
	}
	for (int i = 0; i < size; i++) {
		if (_obs[i] != NULL) {
			continue;
		}
		_obs[i] = o;
		_obsNbr++;
		break;
	}
}

void Subject::detach(Observer *o) {
	for (int i = 0; i < size; i++) {
		if (o != _obs[i]) {
			continue;
		}
		_obs[i] = NULL;
		_obsNbr--;
		break;
	}
}

void Subject::notify(Events event) {
	for (int i = 0; i < size; i++) {
		if (_obs[i] == NULL) {
			continue;
		}
		_obs[i]->update(this, event);
	}
}
