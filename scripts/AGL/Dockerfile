FROM ubuntu:22.04

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install -y build-essential chrpath \
	cpio debianutils diffstat file gawk gcc git iputils-ping libacl1 liblz4-tool \
	locales python3 python3-git python3-jinja2 python3-pexpect \
	python3-pip python3-subunit socat texinfo unzip wget xz-utils zstd curl
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen && locale-gen
RUN curl https://storage.googleapis.com/git-repo-downloads/repo >> /bin/repo && \
	chmod a+x /bin/repo

ENV USER=agl
ENV TZ=Europe/London
RUN adduser --disabled-password --gecos '' $USER
COPY AGL_installation.sh .
RUN chmod +x AGL_installation.sh

USER $USER
WORKDIR /home/$USER/AGL

RUN repo init -b trout -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo && \
	repo sync

COPY meta-mylayer ./meta-mylayer

CMD [ "./AGL_installation.sh" ]
