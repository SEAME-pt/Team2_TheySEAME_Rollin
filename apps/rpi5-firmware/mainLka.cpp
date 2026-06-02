#include "ActuatorController.hpp"
#include "ActuatorKuksa.hpp"
#include "CAN.hpp"
#include "ActuatorCAN.hpp"
#include "Utils.hpp"
#include "PurePursuit.hpp"
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

int readFrameFromPipe(FILE *pipe, void *frameData, const size_t size) {
	volatile struct FrameHeader *tmp;
	volatile struct FrameHeader headerBE;
	size_t read;

	// Read Binary Mask header. Contains a constant number, width an height
	read = fread(header, sizeof(struct FrameHeader), 1, pipe);
	tmp = (struct FrameHeader *)header;

	headerBE.frameNbr = ntohl(tmp->frameNbr);
	headerBE.width = ntohs(tmp->width);
	headerBE.heigth = ntohs(tmp->heigth);
	//std::cout << "Header: " << headerBE.frameNbr << " " << headerBE.heigth << " " << headerBE.width << std::endl;
	if (headerBE.heigth != frameH) {
		return (1);
	}
	
	// Read actual Binary Mask
	read = fread(frameData, size, 1, pipe);
	if (read != 1) {
		std::cout << "Read Problem" << std::endl;
		return (1);
	}
	frameCount++;
	return (0);
}

int main() {
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can);
	//Lka lka(400, 0, 250, 960, 390); // Carla Setup
	PurePursuit pp;
	kuksaLib kuksa;
	//CarActuator *car = new ActuatorKuksa(
	//	new ActuatorCAN(can),
	//	kuksa
	//);
	ActuatorController ctrl(car, NULL, &pp, kuksa);

	std::signal(SIGINT, signal_handler);
	pp.attach(&ctrl);

	FILE *pipe = fopen("NamedPipe", "r");
	if (pipe == NULL) {
		std::cout << "Failed to open NamedPipe" << std::endl;
		return (-1);
	}
	cv::namedWindow("WIN", cv::WINDOW_NORMAL);

	while (run.load()) {
		//usleep(50000);
	}
	fclose(pipe);
	cv::destroyAllWindows();

	free(header);
	delete car;
}
