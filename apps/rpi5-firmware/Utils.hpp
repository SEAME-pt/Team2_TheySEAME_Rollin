#pragma once

#include <cstdint>

// Frame Header returned by the AI pipeline
struct FrameHeader {
	uint32_t frameNbr;
	uint16_t heigth;
	uint16_t width;
	float laneScore;
};

struct quadFunc {
	float a;
	float b;
	float c;
};
