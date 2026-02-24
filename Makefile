OFT_IMAGE := openfasttrace
OFT_DOCKERFILE := scripts/OFT/Dockerfile
PWD := $(shell pwd)
BUILD_DIR := reports

AGL := agl-run
AGL_DIR := scripts/AGL
AGL_IMAGE := agl
AGL_BUILD_DIR := build
AGL_DOWNLOADS_DIR := downloads
AGL_CACHE_DIR := sstate-cache
AGL_HOST_BUILD := $(PWD)/$(AGL_DIR)/$(AGL_BUILD_DIR)
AGL_TARGET_BUILD := /home/build/$(AGL_BUILD_DIR)
AGL_HOST_DOWNLOADS := $(PWD)/$(AGL_DIR)/$(AGL_DOWNLOADS_DIR)
AGL_TARGET_DOWNLOADS := /home/build/$(AGL_DOWNLOADS_DIR)
AGL_HOST_CACHE := $(PWD)/$(AGL_DIR)/$(AGL_CACHE_DIR)
AGL_TARGET_CACHE := /home/build/$(AGL_CACHE_DIR)

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
		$(OFT_IMAGE) trace -o html -f /output/trace_report.html src/ requirements/ \
		|| echo "Trace report generated with coverage issues - check trace_report.html"

## Run trace (no HTML)
oft-shell: oft-build | $(BUILD_DIR)
	docker run --rm \
		--user $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/output \
		$(OFT_IMAGE) trace src/ requirements/

oft-xml: oft-build | $(BUILD_DIR)
	docker run --rm \
		--user $(shell id -u):$(shell id -g) \
		-v $(PWD)/$(BUILD_DIR):/output \
		$(OFT_IMAGE) trace -o aspec -f /output/requirements.xml src/ requirements/ \
		|| echo "Relatório XML gerado com problemas de cobertura - verifique trace_report.xml"

view: oft-xml
	@echo "Starting server at http://localhost:8000"
	@cd $(BUILD_DIR) && python3 -m http.server 8000

agl-image-build:
	docker build --build-arg UID=$(shell id -u) --build-arg GID=$(shell id -g) \
		-t $(AGL_IMAGE) $(AGL_DIR)

$(AGL): agl-image-build
	mkdir -p $(AGL_HOST_BUILD) $(AGL_HOST_DOWNLOADS) $(AGL_HOST_CACHE)
	docker run -it \
		-v $(AGL_HOST_BUILD):$(AGL_TARGET_BUILD) \
		-v $(AGL_HOST_DOWNLOADS):$(AGL_TARGET_DOWNLOADS) \
		-v $(AGL_HOST_CACHE):$(AGL_TARGET_CACHE) \
		-e BUILD_DIR=$(AGL_BUILD_DIR) \
		$(AGL_IMAGE) bash

