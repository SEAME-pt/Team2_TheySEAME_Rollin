#pragma once

class RemoteControl {
public:

	RemoteControl();
	~RemoteControl();

	int getfd() const;
	void openRemoteControl();
	void closeRemoteControl();
	void readEvent();

	int axis_x;
	int axis_y;
	int axis_rx;
	int axis_ry;
private:
	int _fd;
};
