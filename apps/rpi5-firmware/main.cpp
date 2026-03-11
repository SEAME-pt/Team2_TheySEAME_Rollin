#include "Evdev.hpp"
#include "CarKuksa.hpp"
#include "RemoteControl.hpp"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>

bool run = true;

void signal_handler(int signal) {
	run = false;
}

int main() {
	struct pollfd fds[2];
	Evdev evdev("/dev/input/event6");
	RemoteControl remote(evdev);
	CarKuksa car(remote);

	std::signal(SIGINT, signal_handler);
	remote.attach(&car);

	fds[0].fd = evdev.getfd();
	fds[0].events = POLLIN;
	while (run) {
		if (poll(fds, 2, 0) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev.readEvent();
			remote.getEvent();
		}
	}
	return (0);
}
