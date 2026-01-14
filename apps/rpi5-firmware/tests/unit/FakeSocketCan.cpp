#include "FakeSocketCan.hpp"

mockCanbittimng mockCanbtm;

int fakeCan_do_stop() {
	return (1);
}

int can_do_stop(const char *name) {
	return (0);
}

int can_do_start(const char *name) {
	return (0);
}

int can_set_ctrlmode(const char *name, struct can_ctrlmode *mode) {
	return (0);
}

int can_get_ctrlmode(const char *name, struct can_ctrlmode *mode) {
	return (0);
}

int can_set_bitrate(const char *name, uint32_t bitrate) {
	mockCanbtm.mockBitrate = bitrate;
	return (0);
}

int can_get_bittiming(const char *name, struct can_bittiming *bt) {
	return (0);
}
