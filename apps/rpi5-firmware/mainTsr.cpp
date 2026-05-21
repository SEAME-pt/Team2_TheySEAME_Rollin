#include "Utils.hpp"
#include "Tsr.hpp"
#include "ActuatorKuksa.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
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

int main() {
    CAN can("can0", 500, 0, 0);
    kuksaLib kuksa;
    CarActuator *car = new ActuatorKuksa(new ActuatorCAN(can), kuksa);
    Tsr tsr(car);
    tsr.applyScaleCalibration(44.0f, 49.0f);
    FILE *pipe = fopen("NamedPipeTsr", "r");
    if (pipe == NULL) {
        std::cout << "Failed to open NamedPipeTsr" << std::endl;
        return (-1);
    }

    std::cout << "NamedPipeTsr opened successfully" << std::endl;

    while (true) {
        std::vector<TsrHeader> detections;  
        readFromPipe(pipe, detections, frameCount, tsr);

        // std::cout << "Detections in frame: " << detections.size() << std::endl;

        if (feof(pipe)) {
            std::cout << "Pipe EOF" << std::endl;
            break;
        }

        for (auto &d : detections) {
            // std::cout << "Dispatching trafficSign=" << d.trafficSign << std::endl;
            tsr.handleTrafficSign(d);
        }

    }

    fclose(pipe);
    delete car;
    return 0;
}