FROM ubuntu:22.04

ARG UID="1000"
ARG GID="1000"
ENV USER=build
ENV TZ=Europe/London
ENV BUILD_DIR=agl
ENV AGL_TOP=/home/$USER/$BUILD_DIR
ENV LOCAL_CONF=$AGL_TOP/conf/local.conf
ENV AGL_FEATURES="agl-app-framework agl-buildstats agl-devel agl-drm-lease agl-ic \
	agl-netboot agl-package-management agl-pipewire agl-selinux"

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone
RUN apt-get update && apt-get install -y build-essential chrpath \
	cpio debianutils diffstat file gawk gcc git iputils-ping libacl1 liblz4-tool \
	locales python3 python3-git python3-jinja2 python3-pexpect \
	python3-pip python3-subunit socat texinfo unzip wget xz-utils zstd curl
RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen && locale-gen
RUN curl https://storage.googleapis.com/git-repo-downloads/repo >> /bin/repo
RUN chmod a+x /bin/repo

RUN groupadd -g $GID AGL
RUN useradd -u $UID -g $GID -m -s /bin/bash $USER
USER $USER
WORKDIR /home/$USER/

RUN git config --global user.email "$USER@example.com" && \
	git config --global user.name "$USER" && \
	git config --global color.ui false

RUN repo init -b trout -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo && \
	repo sync

COPY conf .
COPY layers .

CMD [ "bash", "installAGL.sh" ]
