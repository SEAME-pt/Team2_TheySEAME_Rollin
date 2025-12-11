#pragma once

class RemoteControl {
public:

	RemoteControl();
	~RemoteControl();

	void openRemoteControl();
private:
	int _fd;
};
