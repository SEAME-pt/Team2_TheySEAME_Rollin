#include "Utils.hpp"
#include "Tsr.hpp"
#include "ActuatorKuksa.hpp"
#include "ActuatorCAN.hpp"
#include "CAN.hpp"
#include <arpa/inet.h>
#include <iostream>

int frameCount = 0;
void *header = malloc(sizeof(struct TsrHeader));


void readFromPipe(FILE *pipe, std::vector<TsrHeader> &detections, int &frameCount)
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
        std::cout << "Sync Problem (got " << frameNbr << ")" << std::endl;
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
    

    // for (int i = 1; i < numDetections; i++) {
    //     if (fread(&raw, sizeof(TsrHeader), 1, pipe) != 1) {
    //         std::cout << "Failed to read detection " << i << std::endl;
    //         return;
    //     }
    //     std::cout << "Trafic Sign: " << detections[i].trafficSign << std::endl;
    //     detections.push_back(decode(raw));
    // }

    frameCount++;
}

int main() {
    CAN can("can0", 500, 0, 0);
    kuksaLib kuksa;
    CarActuator *car = new ActuatorKuksa(new ActuatorCAN(can), kuksa);
    Tsr tsr(car);

    FILE *pipe = fopen("NamedPipeTsr", "r");
    if (pipe == NULL) {
        std::cout << "Failed to open NamedPipeTsr" << std::endl;
        return (-1);
    }

    while (true) {
        std::vector<TsrHeader> detections;
        readFromPipe(pipe, detections, frameCount);

        if (feof(pipe)) {
            std::cout << "Pipe EOF" << std::endl;
            break;
        }

        for (auto &d : detections) {
            std::cout << "Trafic Sign: " << d.trafficSign << std::endl;
            tsr.handleTrafficSign(d);
        }

    }

    fclose(pipe);
    delete car;
    return 0;
}