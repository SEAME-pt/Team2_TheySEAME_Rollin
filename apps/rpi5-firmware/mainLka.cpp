#include "ActuatorController.hpp"
#include "ActuatorKuksa.hpp"
#include "CAN.hpp"
#include "ActuatorCAN.hpp"
#include "Utils.hpp"
#include "Lka.hpp"
#include <csignal>
#include <arpa/inet.h>

std::atomic<bool> run = true;
const int frameH = 640;
const int frameW = 640;
int frameCount = 1;
void *header = malloc(sizeof(struct FrameHeader));

void signal_handler(int signal) {
	run.store(false);
}

void readFrameFromPipe(FILE *pipe, void *frameData, const size_t size) {
	volatile struct FrameHeader *tmp;
	volatile struct FrameHeader headerBE;
	size_t read;

	// Read Binary Mask header. Contains a constant number, width an height
	read = fread(header, sizeof(struct FrameHeader), 1, pipe);
	tmp = (struct FrameHeader *)header;

	headerBE.frameNbr = ntohl(tmp->frameNbr);
	headerBE.width = ntohs(tmp->width);
	headerBE.heigth = ntohs(tmp->heigth);
	std::cout << "Frame Number: " << headerBE.frameNbr << " " << frameCount << std::endl;
	std::cout << "Size: " << headerBE.heigth << " " << headerBE.width << std::endl;
	if (headerBE.frameNbr != 88000) {
		std::cout << "Sync Problem" << std::endl;
		return;
	}
	
	// Read actual Binary Mask
	read = fread(frameData, size, 1, pipe);
	if (read != 1) {
		std::cout << "Read Problem" << std::endl;
		return;
	}
	frameCount++;
}

int handleFrame(FILE *pipe, Lka &lka) {
	Frame frame(frameH, frameW, BINARY_FRAME);

	readFrameFromPipe(pipe, frame.getRawData(), frameH * frameW);

	lka.poly(frame);

	return (0);
}

int main() {
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can);
	//Lka lka(400, 0, 250, 960, 390); // Carla Setup
	Lka lka(150, 0, 180, frameW, frameH - 180, 8);
	kuksaLib kuksa;
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can),
	//	kuksa
	//);
	ActuatorController ctrl(car, NULL, &lka, kuksa);

	std::signal(SIGINT, signal_handler);
	lka.attach(&ctrl);

	FILE *pipe = fopen("NamedPipe", "r");
	if (pipe == NULL) {
		std::cout << "Failed to open NamedPipe" << std::endl;
		return (-1);
	}
	cv::namedWindow("WIN", cv::WINDOW_NORMAL);

	while (run.load()) {
		//usleep(50000);
		if (handleFrame(pipe, lka) == -1) {
			break;
		}
	}
	fclose(pipe);
	cv::destroyAllWindows();

	free(header);
	delete car;
}
