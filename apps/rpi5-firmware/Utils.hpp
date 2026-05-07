#pragma once

struct FrameHeader {
	uint32_t frameNbr;
	uint16_t heigth;
	uint16_t width;
	float laneScore;
};

struct TsrHeader {
    uint32_t frameNbr;
    uint16_t numDetections;
    int      trafficSign;
    float    accuracy;
    int      x;
    int      y;
    int      width;
    int      height;
};

