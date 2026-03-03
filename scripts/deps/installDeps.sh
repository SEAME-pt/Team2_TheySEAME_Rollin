# Install Protobuf 25.8
git clone -b v25.8 --recurse-submodules git@github.com:protocolbuffers/protobuf.git
mkdir build && cd build
cmake ../ \
	-DCMAKE_POSITION_INDEPENDENT_CODE=ON \
    -DCMAKE_BUILD_TYPE=Release \
    -Dprotobuf_BUILD_TESTS=OFF
make -j 16
make install

# Install gRPC 
git clone -b v1.60.1 --recurse-submodules --depth 1 https://github.com/grpc/grpc \
cd grpc && mkdir build && cd build
cmake .. \
       -DgRPC_INSTALL=ON \
       -DgRPC_BUILD_TESTS=OFF \
       -DCMAKE_BUILD_TYPE=Release \
       -DgRPC_ABSL_PROVIDER=package \
       -DgRPC_PROTOBUF_PROVIDER=package \
       -DgRPC_SSL_PROVIDER=package \
       -DgRPC_ZLIB_PROVIDER=package
make -j 16
make install
