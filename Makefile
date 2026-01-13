OFT_IMAGE := openfasttrace
OFT_DOCKERFILE := scripts/OFT/Dockerfile
PWD := $(shell pwd)
BUILD_DIR := reports

.PHONY: oft oft-build oft-html all

all: oft

## Build the OpenFastTrace Docker image
oft-build:
	docker build -f $(OFT_DOCKERFILE) -t $(OFT_IMAGE) .

## Ensure output directory exists
$(BUILD_DIR):
	mkdir -p $@
	chmod 755 $@

## Generate HTML trace report
oft: oft-build | $(BUILD_DIR)
	docker run --rm \
		--user $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/output \
		$(OFT_IMAGE) trace -o aspec -o html -f /output/trace_report.html src/ requirements/ \
		|| echo "Trace report generated with coverage issues - check trace_report.html"

## Run trace (no HTML)
oft-shell: oft-build | $(BUILD_DIR)
	docker run --rm \
		--user $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/output \
		$(OFT_IMAGE) trace src/ requirements/