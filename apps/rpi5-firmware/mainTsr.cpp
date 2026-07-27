#include "Utils.hpp"
#include "Tsr.hpp"
#include "ActuatorKuksa.hpp"
#include "ActuatorCAN.hpp"
#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "HazardDetector.hpp"
#include <json/json.h>
#include <mqtt/async_client.h>
#include <arpa/inet.h>
#include <iostream>

int frameCount = 0;
void *header = malloc(sizeof(struct TsrHeader));

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

int main() {
    // CAN can("can0", 500, 0, 0);
    kuksaLib kuksa;
    CarActuator *car = new ActuatorKuksa(nullptr, kuksa);
    Tsr tsr;
    ActuatorController controller(car, nullptr, nullptr, kuksa, &tsr);
    tsr.attach(&controller);
    
    tsr.resetKuksa();

    //mqtt
    mqtt::async_client mqtt("tcp://10.21.100.3:1883", "tsr_publisher");

    mqtt::connect_options connOpts;
    connOpts.set_clean_session(true);
    connOpts.set_connect_timeout(std::chrono::seconds(5));
    connOpts.set_automatic_reconnect(true);

    try {
        std::cout << "[MQTT] Connecting..." << std::endl;
        mqtt.connect(connOpts)->wait();
        std::cout << "[MQTT] Connected to broker" << std::endl;
    } catch (const mqtt::exception& e) {
        std::cerr << "[MQTT] Connect failed: " << e.what() << std::endl;
    }
	HazardDetector::Config hazardCfg;
    HazardDetector hazardDetector(hazardCfg);
    HazardType lastPublishedHazard = HazardType::NONE;
    uint32_t lastPublishedMarkerId = 0;

    //pipe
    FILE *pipe = fopen("NamedPipeTsr", "r");
    if (pipe == NULL) {
        std::cout << "Failed to open NamedPipeTsr" << std::endl;
        return (-1);
    }

    std::cout << "NamedPipeTsr opened successfully" << std::endl;
    HazardResult hazard;

    //kuksaLib thread
    std::thread vhState(&kuksaLib::subscribeFromKuksa, &kuksa);
    while (true) {
        std::vector<TsrHeader> detections;  
        readFromPipe(pipe, detections, frameCount, tsr);

        // std::cout << "Detections in frame: " << detections.size() << std::endl;

        if (feof(pipe)) {
            std::cout << "Pipe EOF" << std::endl;
            break;
        }
        tsr.clearDetectedSigns();
        hazardDetector.setOurSpeed(kuksa.getSpeed());
        tsr.setMainTsr(true);
        for (auto &d : detections) {
            kuksa.sendValueToKuksa("mobility_scenario.hazard.marker_id", d.marker_id);
            tsr.handleTrafficSign(d);
            hazardDetector.update(d);
        }
        hazard = hazardDetector.evaluate();
        if (hazard.hazard != HazardType::NONE) {
            bool isNewHazard = hazard.hazard != lastPublishedHazard || hazard.marker_id != lastPublishedMarkerId;
            if (!isNewHazard) {
                hazardDetector.endFrame();
                tsr.tick();
                continue;
            }

            if (hazard.hazard == HazardType::STOPPED_CAR) {
				publish("stopped_car", hazard.marker_id, mqtt);       
			}
            else if (hazard.hazard == HazardType::OBJECT_ON_TRACK) {
				publish("stopped_obstacles", hazard.marker_id, mqtt);
			}

            lastPublishedHazard = hazard.hazard;
            lastPublishedMarkerId = hazard.marker_id;
        } else {
            lastPublishedHazard = HazardType::NONE;
            lastPublishedMarkerId = 0;
        }
		hazardDetector.endFrame();
        tsr.tick();

    }
    vhState.join();
    fclose(pipe);
    delete car;
    return 0;
}