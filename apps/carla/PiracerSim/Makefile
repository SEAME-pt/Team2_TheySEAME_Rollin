CARLADIR=$(CURDIR)/..
BUILDDIR=$(CURDIR)/build
BINDIR=$(CURDIR)/bin
INSTALLDIR=$(CURDIR)/libcarla-install
TOOLCHAIN=$(CURDIR)/ToolChain.cmake
KUKSADIR=$(CURDIR)/kuksa
LIBKUKSADIR=$(CURDIR)/libkuksa
KUKSABUILDDIR=$(LIBKUKSADIR)/build
OPENCV_LIBS  := $(shell pkg-config --libs opencv4)
OPENCV_FLAGS := $(shell pkg-config --cflags opencv4)

CC=/usr/bin/gcc
CXX=/usr/bin/g++
CXXFLAGS=-std=c++17 -pthread -fPIC -O3 -DNDEBUG

ABSL_DIR := /usr/local/lib
ABSL_LIBS := $(wildcard $(ABSL_DIR)/libabsl_*.a)

define log
	@echo "\033[1;35m$(1)\033[0m"
endef

default: build

clean:
	@rm -rf $(BUILDDIR) $(INSTALLDIR)
	@rm -f ToolChain.cmake

run: build
	$(call log,Running C++ Client...)
	@$(BINDIR)/cpp_client $(ARGS)

run.only:
	$(call log,Running C++ Client...)
	@$(BINDIR)/cpp_client $(ARGS)

run.only-auto:
	$(call log,Running C++ Client [AUTOPILOT]...)
	@$(BINDIR)/cpp_client --auto

run.only-manual:
	$(call log,Running C++ Client [MANUAL]...)
	@$(BINDIR)/cpp_client --manual

re: 
	rm -rf $(BINDIR)/cpp_client
	$(MAKE) build

build: $(BINDIR)/cpp_client

$(BINDIR)/cpp_client: | build_libcarla build_libkuksa
	$(call log,Compiling C++ Client...)
	@mkdir -p $(BINDIR)
	@$(CXX) $(CXXFLAGS) $(OPENCV_FLAGS) -I . -I$(INSTALLDIR)/include -isystem $(INSTALLDIR)/include/system -L$(INSTALLDIR)/lib \
		-o $(BINDIR)/cpp_client src/main.cpp \
		$(KUKSADIR)/val/v2/*.cc $(KUKSADIR)/val/v2/*.cpp \
		-Wl,-Bstatic -lcarla_client -lrpc -lboost_filesystem -Wl,-Bdynamic \
		-L/usr/local/lib -L/usr/lib/x86_64-linux-gnu \
		-Wl,--start-group \
		-lgrpc++ -lgrpc -lgpr -lprotobuf \
		-l:libcares.a \
		-l:libaddress_sorting.a \
		-l:libre2.a \
		-l:libupb.a \
		-l:libupb_collections_lib.a \
		-l:libupb_json_lib.a \
		-l:libupb_textformat_lib.a \
		-l:libutf8_range.a \
		-l:libutf8_range_lib.a \
		-l:libutf8_validity.a \
		$(ABSL_LIBS) \
		-Wl,--end-group \
		-lz -lssl -lcrypto -ldl -lpthread \
		-lpng -ltiff -ljpeg -lRecast -lDetour -lDetourCrowd \
		$(OPENCV_LIBS)
		
build_libcarla: $(TOOLCHAIN)
	@cd $(CARLADIR); make setup
	@mkdir -p $(BUILDDIR)
	$(call log,Compiling LibCarla.client...)
	@{ \
		cd $(BUILDDIR); \
		if [ ! -f "build.ninja" ]; then \
		cmake \
			-G "Ninja" \
			-DCMAKE_BUILD_TYPE=Client \
			-DLIBCARLA_BUILD_RELEASE=ON \
			-DLIBCARLA_BUILD_DEBUG=OFF \
			-DLIBCARLA_BUILD_TEST=OFF \
			-DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) \
			-DCMAKE_INSTALL_PREFIX=$(INSTALLDIR) \
			-DCMAKE_EXPORT_COMPILE_COMMANDS=1 \
			-DCMAKE_CXX_FLAGS="-Wno-error" \
			$(CARLADIR); \
	  fi; \
		ninja; \
		ninja install | grep -v "Up-to-date:"; \
	}

build_libkuksa:
	$(call log,Compiling libkuksa...)
	@mkdir -p $(KUKSABUILDDIR)
	@cd $(KUKSABUILDDIR) && cmake .. && $(MAKE)
	@cp -r $(KUKSABUILDDIR)/kuksa ./
	@cp $(LIBKUKSADIR)/KuksaLib.cpp $(KUKSADIR)/val/v2/
	@cp $(LIBKUKSADIR)/KuksaLib.hpp $(KUKSADIR)/val/v2/

$(TOOLCHAIN):
	@echo "set(CMAKE_C_COMPILER $(CC))" > $(TOOLCHAIN)
	@echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(TOOLCHAIN)
	@echo "set(CMAKE_CXX_FLAGS \"\$${CMAKE_CXX_FLAGS} $(CXXFLAGS)\" CACHE STRING \"\" FORCE)" >> $(TOOLCHAIN)
