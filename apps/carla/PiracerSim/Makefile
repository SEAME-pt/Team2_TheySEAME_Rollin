CARLADIR=$(CURDIR)/..
BUILDDIR=$(CURDIR)/build
BINDIR=$(CURDIR)/bin
INSTALLDIR=$(CURDIR)/libcarla-install
TOOLCHAIN=$(CURDIR)/ToolChain.cmake

CC=/usr/bin/gcc
CXX=/usr/bin/g++
CXXFLAGS=-std=c++17 -pthread -fPIC -O3 -DNDEBUG

define log
	@echo "\033[1;35m$(1)\033[0m"
endef

default: build

clean:
	@rm -rf $(BUILDDIR) $(INSTALLDIR)
	@rm -f ToolChain.cmake

run: build
	$(call log,Running C++ Client...)
	@$(BINDIR)/piracer_sim $(ARGS)

run.only:
	$(call log,Running C++ Client...)
	@$(BINDIR)/piracer_sim $(ARGS)

re: 
	rm -rf $(BINDIR)/piracer_sim
	$(MAKE) build

build: $(BINDIR)/piracer_sim

$(BINDIR)/piracer_sim: | build_libcarla
	$(call log,Compiling C++ Client...)
	@mkdir -p $(BINDIR)
	@$(CXX) $(CXXFLAGS) -I$(INSTALLDIR)/include -isystem $(INSTALLDIR)/include/system -L$(INSTALLDIR)/lib \
		-o $(BINDIR)/piracer_sim src/main.cpp \
		kuksa/val/v2/*.cc kuksa/val/v2/*.cpp \
		-Wl,-Bstatic -lcarla_client -lrpc -lboost_filesystem -Wl,-Bdynamic \
		-lgrpc++ -lgrpc -lgpr -lprotobuf -lz -lssl -lcrypto -ldl \
		-lpng -ltiff -ljpeg -lRecast -lDetour -lDetourCrowd

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

$(TOOLCHAIN):
	@echo "set(CMAKE_C_COMPILER $(CC))" > $(TOOLCHAIN)
	@echo "set(CMAKE_CXX_COMPILER $(CXX))" >> $(TOOLCHAIN)
	@echo "set(CMAKE_CXX_FLAGS \"\$${CMAKE_CXX_FLAGS} $(CXXFLAGS)\" CACHE STRING \"\" FORCE)" >> $(TOOLCHAIN)
