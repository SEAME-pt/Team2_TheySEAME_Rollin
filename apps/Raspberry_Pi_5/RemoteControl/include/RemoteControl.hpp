#pragma once

class RemoteControl {
public:

	RemoteControl();
	~RemoteControl();

	int getfd() const;
	void openRemoteControl();
	void closeRemoteControl();
	void readEvent();

private:
	int _fd;
};
