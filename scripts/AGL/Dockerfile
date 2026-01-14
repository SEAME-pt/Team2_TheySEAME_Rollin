FROM ubuntu:22.04

ENV AGL_TOP=/AGL
ENV USER=agl
WORKDIR $AGL_TOP

ENV TZ=Europe/London
ENV BUILD_DIR=build
ENV LOCAL_CONF=$AGL_TOP/$BUILD_DIR/conf/local.conf
ENV AGL_FEATURES="agl-app-framework agl-buildstats agl-devel agl-drm-lease agl-ic \
	agl-netboot agl-package-management agl-pipewire agl-selinux"
ENV QT_RECIPES=" qtbase qtmultimedia qtquick3d qtdeclarative"
ENV DEVEL_RECIPES=" cmake"

RUN ln -snf /usr/share/zoneinfo/$TZ /etc/localtime && echo $TZ > /etc/timezone

RUN apt-get update && apt-get install -y build-essential chrpath \
	cpio debianutils diffstat file gawk gcc git iputils-ping libacl1 liblz4-tool \
	locales python3 python3-git python3-jinja2 python3-pexpect \
	python3-pip python3-subunit socat texinfo unzip wget xz-utils zstd curl

RUN echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen && locale-gen

RUN curl https://storage.googleapis.com/git-repo-downloads/repo >> /bin/repo && \
	chmod a+x /bin/repo

RUN repo init -b trout -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo && \
	repo sync

COPY AGL_installation.sh .

RUN chmod +x AGL_installation.sh

RUN adduser --disabled-password --gecos '' $USER && chown $USER:$USER $AGL_TOP

USER $USER

ENTRYPOINT [ "./AGL_installation.sh" ]
