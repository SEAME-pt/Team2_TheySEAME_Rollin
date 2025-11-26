#include <periphery/spi.h>
#include <stdio.h>
#include <stdlib.h>

int main() {
	spi_t *spi;
	const char *path = "/dev/spidev0.1";

	spi = spi_new();

	printf("Opening %s\n", path);
	if (spi_open(spi, path, 0, 10000000) < 0) {
		printf("Error trying to open SPI: %s\n", spi_errmsg(spi));
		exit(1);
	}
	printf("Open %s successfully\n", path);

	printf("Closing %s device\n", path);
	spi_close(spi);

	spi_free(spi);

	return (0);
}
