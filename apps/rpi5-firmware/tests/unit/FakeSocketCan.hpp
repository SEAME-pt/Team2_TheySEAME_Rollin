#include <libsocketcan.h>
#include <cstdint>

struct mockCanbittimng {
	uint32_t mockBitrate;
};

extern mockCanbittimng mockCanbtm;

int fakeCan_do_stop();
