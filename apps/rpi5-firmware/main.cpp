#include "Evdev.hpp"
#include "RemoteControl.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include "ActuatorKuksa.hpp"
#include "HazardDetector.hpp"
#include "Tsr.hpp"
#include <unistd.h>
#include <stdlib.h>
#include <poll.h>
#include <csignal>
#include <stdio.h>
#include <thread>
#include "ActuatorController.hpp"
#include <opencv4/opencv2/highgui.hpp>
#include <json/json.h>
#include <mqtt/async_client.h>

int frameCount = 0;
void *header = malloc(sizeof(struct TsrHeader));
Frame show;

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

void publish(const std::string& type, uint32_t marker_id, mqtt::async_client &mqtt)
{
    if (!mqtt.is_connected()) {
        std::cerr << "[MQTT] Not connected, dropping publish: " << type << std::endl;
        return;
    }

    Json::Value root;
    root["marker_id"] = marker_id;
    root["type"] = type;

    Json::StreamWriterBuilder builder;
    std::string json = Json::writeString(builder, root);

    try {
        auto msg = mqtt::make_message("/incidents", json);
        msg->set_qos(0);
        mqtt.publish(msg);
    } catch (const mqtt::exception& e) {
        std::cerr << "[MQTT] Publish failed: " << e.what() << std::endl;
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
		d.marker_id     = ntohl(r.marker_id);
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
        // std::cout << "header: frameNbr=" << detections.back().frameNbr
        //           << " numDetections=" << detections.back().numDetections
        //           << " trafficSign=" << detections.back().trafficSign
        //           << " accuracy=" << detections.back().accuracy
        //           << " x=" << detections.back().x
        //           << " y=" << detections.back().y
        //           << " width=" << detections.back().width
        //           << " height=" << detections.back().height
        //           << std::endl;
    }

    frameCount++;
}

void tsrThread(Tsr *tsr, kuksaLib *kuksa) {
	mqtt::async_client mqtt("tcp://10.21.100.2:1883", "tsr_publisher");
    mqtt.connect();
	HazardDetector::Config hazardCfg;
    HazardDetector hazardDetector(hazardCfg);
    tsr->resetKuksa();
	hazardDetector.setOurSpeed(kuksa->getSpeed());
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
            hazardDetector.update(d);
        }
		HazardResult hazard = hazardDetector.evaluate();
		std::cout << "Hazard detected: " << static_cast<int>(hazard.hazard) << ", marker_id: " << hazard.marker_id << std::endl;
        if (hazard.hazard != HazardType::NONE) {
            if (hazard.hazard == HazardType::STOPPED_CAR) {
				publish("stopped_car", hazard.marker_id, mqtt);       
			} else if (hazard.hazard == HazardType::OBJECT_ON_TRACK) {
				std::cout << "Publishing object_on_track with marker_id: " << hazard.marker_id << std::endl;
				publish("stopped_obstacles", hazard.marker_id, mqtt);
			}
			else if (hazard.hazard == HazardType::TWO_STOPPED_CARS) {
				publish("stopped_car", hazard.marker_id, mqtt);
			}
        }
		hazardDetector.endFrame();
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
	
	std::thread TsrThread(tsrThread, &tsr, &kuksa);
	// Kuksa Thread
	// std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);

	while (run.load()) {
		usleep(50000);
		ctrl.test();
	}

	//lkaThread.join();
	remoteThread.join();

	//tsrThread 
	// TsrThread.join();

	//vhState.join();
	delete car;

	return (0);
}
