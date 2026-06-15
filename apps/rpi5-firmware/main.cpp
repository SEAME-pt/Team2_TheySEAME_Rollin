#include "Evdev.hpp"
#include "RemoteControl.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include "ActuatorKuksa.hpp"
#include "Tsr.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>
#include <stdio.h>
#include <thread>
#include "ActuatorController.hpp"
#include <opencv4/opencv2/highgui.hpp>

int frameCount = 0;
void *header = malloc(sizeof(struct TsrHeader));

std::atomic<bool> run = true;

void signal_handler(int signal) {
	run.store(false);
}

int handleFrame(cv::VideoCapture &cam, Lka &lka) {
	int i = 0;
	
	cv::Mat frameRaw;
	cam.read(frameRaw);
	if (frameRaw.empty()) {
		std::cout << "Failed to get Frame" << std::endl;
		return (-1);
	}
	Frame frame(frameRaw);

	lka.poly(frame);
	return (0);
}

void pathPlanning(Lka *lka) {
	cv::namedWindow("WIN", cv::WINDOW_NORMAL);
	cv::moveWindow("WIN", 0, 0);
	cv::VideoCapture cam("pipe:0");
	//cam.set(cv::CAP_PROP_FPS, 20);
	if (!cam.isOpened()) {
		std::cout << "Didnt open" << std::endl;
		return;
	}
	while (run.load()) {
		//usleep(50000);
		if (handleFrame(cam, *lka) == -1) {
			break;
		}
	}
	cam.release();
	cv::destroyAllWindows();
}

void remoteControl(RemoteControl *remote, Evdev *evdev) {
	struct pollfd fds[1];

	fds[0].fd = evdev->getfd();
	fds[0].events = POLLIN;
	while (run.load()) {
		if (poll(fds, 1, 100) < 0) {
			perror("Error in poll:");
			break;
		}
		if (fds[0].revents & POLLIN) {
			evdev->readEvent();
			remote->getEvent();
		}
	}
}


void readFromPipe(FILE *pipe, std::vector<TsrHeader> &detections, int &frameCount, Tsr &tsr)
{
    TsrHeader raw;
    TsrHeader headerBE;

    if (fread(&raw, sizeof(TsrHeader), 1, pipe) != 1) {
        std::cout << "Pipe closed or read error" << std::endl;
        return;
    }

    uint32_t frameNbr      = ntohl(raw.frameNbr);
    uint16_t numDetections = ntohs(raw.numDetections);

    if (frameNbr != FRAME_NMBR) {
        std::cout << "Sync Problem (got " << frameNbr << ", expected " << FRAME_NMBR << ")" << std::endl;
        return;
    }

    auto decode = [](const TsrHeader &r) -> TsrHeader {
        TsrHeader d;
        d.frameNbr      = ntohl(r.frameNbr);
        d.numDetections = ntohs(r.numDetections);
        d.trafficSign   = ntohs(r.trafficSign);
        d.x             = ntohl(r.x);
        d.y             = ntohl(r.y);
        d.width         = ntohl(r.width);
        d.height        = ntohl(r.height);
        uint32_t accRaw = ntohl(*(uint32_t *)&r.accuracy);
        memcpy(&d.accuracy, &accRaw, sizeof(float));
        return d;
    };

    detections.push_back(decode(raw));
    
    
    TsrHeader decoded;
    for (int i = 1; i < numDetections; i++) {
        if (fread(&raw, sizeof(TsrHeader), 1, pipe) != 1) {
            std::cout << "Failed to read detection " << i << std::endl;
            return;
        }
        decoded = decode(raw);
        detections.push_back(decoded);
        std::cout << "header: frameNbr=" << detections.back().frameNbr
                  << " numDetections=" << detections.back().numDetections
                  << " trafficSign=" << detections.back().trafficSign
                  << " accuracy=" << detections.back().accuracy
                  << " x=" << detections.back().x
                  << " y=" << detections.back().y
                  << " width=" << detections.back().width
                  << " height=" << detections.back().height
                  << std::endl;
    }

    frameCount++;
}

void tsrThread(Tsr *tsr) {
    tsr->resetKuksa();
    FILE *pipe = fopen("NamedPipeTsr", "r");
    if (pipe == NULL) {
        std::cout << "Failed to open NamedPipeTsr" << std::endl;
        return;
    }

    std::cout << "NamedPipeTsr opened successfully" << std::endl;

    while (true) {
        std::vector<TsrHeader> detections;  
        readFromPipe(pipe, detections, frameCount, *tsr);

        if (feof(pipe)) {
            std::cout << "Pipe EOF" << std::endl;
            break;
        }
        tsr->clearDetectedSigns();
        for (auto &d : detections) {
            tsr->handleTrafficSign(d);
        }
        tsr->tick();
    }

    fclose(pipe);
}

int main() {
	Evdev evdev("/dev/input/event4");
	RemoteControl remote(evdev);
	CAN can("can0", 500, 0, 0);
	kuksaLib kuksa;
	CarActuator *car = new ActuatorCAN(can);
	// CarActuator *car = new ActuatorKuksa(
	// 	new ActuatorCAN(can),
	// 	kuksa
	// );
	//Lka lka(400, 0, 250, 960, 390); // Carla Setup
	Lka lka(400, 0, 400, 1536, 464, 8); // Track Setup
	Tsr tsr;
	ActuatorController ctrl(car, &remote, &lka, kuksa, &tsr);

	lka.attach(&ctrl);
	remote.attach(&ctrl);

	tsr.attach(&ctrl);

	std::signal(SIGINT, signal_handler);

	//std::thread lkaThread(pathPlanning, &lka);
	std::thread remoteThread(remoteControl, &remote, &evdev);
	
	std::thread TsrThread(tsrThread, &tsr);
	// Kuksa Thread
	// std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	//while (run.load()) {
	//	usleep(50000);
	//	ctrl.test();
	//}

	//lkaThread.join();
	remoteThread.join();

	//tsrThread 
	// TsrThread.join();

	//vhState.join();
	delete car;

	return (0);
}
