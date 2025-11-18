FROM ubuntu:22.04

RUN apt-get update && apt-get install -y wget
RUN apt-get install -y lsb-release software-properties-common gnupg
RUN wget https://apt.llvm.org/llvm.sh
RUN chmod +x llvm.sh
RUN ./llvm.sh 22
RUN apt-get update && apt-get install --no-install-recommends -y clang-format-22

ENTRYPOINT [ "clang-format-22" ]
