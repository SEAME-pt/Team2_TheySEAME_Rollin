#pragma once
#include <cstdint>

struct FrameHeader {
	uint32_t frameNbr;
	uint16_t heigth;
	uint16_t width;
	float laneScore;
};

struct TsrHeader {
    uint32_t	frameNbr;
    uint16_t	numDetections;
    uint16_t    trafficSign;
    float		accuracy;
    uint32_t    x;
    uint32_t    y;
    uint32_t    width;
    uint32_t    height;
};

