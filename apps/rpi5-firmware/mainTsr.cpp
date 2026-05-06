#include "ActuatorKuksa.hpp"
#include "CAN.hpp"
#include "ActuatorCAN.hpp"
#include "Utils.hpp"
#include "Tsr.hpp"
#include <csignal>
#include <arpa/inet.h>

int main() {
	CAN can("can0", 500, 0, 0);
	CarActuator *car = new ActuatorCAN(can);

	kuksaLib kuksa;
    Tsr tsr;

	FILE *pipe = fopen("NamedPipe", "r");
	if (pipe == NULL) {
		std::cout << "Failed to open NamedPipe" << std::endl;
		return (-1);
	}
	fclose(pipe);

	delete car;
	delete &tsr;
}