#include "Utils.hpp"
#include "Tsr.hpp"
#include "ActuatorKuksa.hpp"
#include "ActuatorCAN.hpp"
#include "ActuatorController.hpp"
#include "CAN.hpp"
#include "HazardDetector.hpp"
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
    }
    frameCount++;
}

static const char* hazardName(HazardType h)
{
    switch (h) {
        case HazardType::NONE:             return "NONE";
        case HazardType::STOPPED_CAR:      return "STOPPED_CAR";
        case HazardType::TWO_STOPPED_CARS: return "TWO_STOPPED_CARS";
        case HazardType::OUR_CAR_STOPPED:  return "OUR_CAR_STOPPED";
        case HazardType::OBJECT_ON_TRACK:  return "OBJECT_ON_TRACK";
    }
    return "?";
}

int main() {
    kuksaLib kuksa;
    Tsr tsr;

    HazardDetector::Config hazardCfg;
    HazardDetector hazardDetector(hazardCfg);

    tsr.resetKuksa();
    FILE *pipe = fopen("NamedPipeTsr", "r");
    if (pipe == NULL) {
        std::cout << "Failed to open NamedPipeTsr" << std::endl;
        return (-1);
    }
    std::cout << "NamedPipeTsr opened successfully" << std::endl;
    while (true) {
        std::vector<TsrHeader> detections;
        readFromPipe(pipe, detections, frameCount, tsr);
        if (feof(pipe)) {
            std::cout << "Pipe EOF" << std::endl;
            break;
        }

        // TODO: substituir por kuksa.getSpeed() (ou equivalente) quando
        // o Vehicle.Speed estiver acessível via kuksaLib. Por agora a
        // deteção de "nosso carro parado/movimento" não tem fonte real.
        hazardDetector.setOurSpeed(0.0f);

        tsr.clearDetectedSigns();
        for (auto &d : detections) {
            tsr.handleTrafficSign(d);
            hazardDetector.update(d);
        }

        HazardResult hazard = hazardDetector.evaluate();
        if (hazard.hazard != HazardType::NONE) {
            std::cout << "[HAZARD] " << hazardName(hazard.hazard)
                       << " - " << hazard.description << std::endl;
            // TODO: publish via MQTT (Mosquitto)
        }
        hazardDetector.endFrame();

        tsr.tick();
    }
    fclose(pipe);
    delete car;
    return 0;
}