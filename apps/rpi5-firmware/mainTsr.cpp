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
    volatile struct TsrHeader *tmp;
    TsrHeader headerBE;
    size_t read;

    read = fread(header, sizeof(struct TsrHeader), 1, pipe);
    if (read != 1) {
        std::cout << "Pipe closed or read error" << std::endl;
        return;
    }

    tmp = (struct TsrHeader *)header;

    headerBE.frameNbr      = ntohl(tmp->frameNbr);
    headerBE.numDetections = ntohs(tmp->numDetections);
    headerBE.trafficSign   = ntohl(tmp->trafficSign);
    headerBE.x             = ntohl(tmp->x);
    headerBE.y             = ntohl(tmp->y);
    headerBE.width         = ntohl(tmp->width);
    headerBE.height        = ntohl(tmp->height);

    uint32_t accRaw = ntohl(*(uint32_t *)&tmp->accuracy);
    memcpy(&headerBE.accuracy, &accRaw, sizeof(float));

    std::cout << "Frame: "  << headerBE.frameNbr
              << " Sign: "  << headerBE.trafficSign
              << " Acc: "   << headerBE.accuracy
              << " Dets: "  << headerBE.numDetections << std::endl;

    frameCount++;

    detections.push_back(headerBE);
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

        for (auto &d : detections)
            tsr.handleTrafficSign(d);
    }

    fclose(pipe);
    delete car;
    return 0;
}