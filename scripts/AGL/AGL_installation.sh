BIN=$HOME/bin
AGL_TOP=$HOME/AGL
AGL_BRANCH=trout
BUILD_DIR=build
LOCAL_CONF=$AGL_TOP/$AGL_BRANCH/$BUILD_DIR/conf/local.conf
QT_RECIPES=" qtbase qtmultimedia qtquick3d qtdeclarative"
DEVEL_RECIPES=" cmake"

# Install Host required packages
sudo apt-get install -y build-essential chrpath cpio debianutils diffstat file gawk gcc \
	git iputils-ping libacl1 liblz4-tool locales python3 python3-git python3-jinja2 python3-pexpect \
	python3-pip python3-subunit socat texinfo unzip wget xz-utils zstd

if [[ -z $(locale --all-locales | grep en_US.utf8) ]]; then
	echo "No locale"
	echo "en_US.UTF-8 UTF-8" >> /etc/locale.gen
	locale-gen
fi
# If possible check the disk space
# Check the git, python and tar versions
sudo apt-get -y install curl

# Set up the "repo" tool
mkdir -p $BIN
export PATH=$BIN:$PATH
if [ ! -x $BIN/repo ]; then
	echo "No repo found"
	curl https://storage.googleapis.com/git-repo-downloads/repo > $BIN/repo
	chmod a+x $BIN/repo
fi

# Download AGL files
mkdir -p $AGL_TOP
cd $AGL_TOP
mkdir -p $AGL_BRANCH
cd $AGL_BRANCH
repo init -b $AGL_BRANCH -u https://gerrit.automotivelinux.org/gerrit/AGL/AGL-repo
repo sync

# Init the build enviroment
source meta-agl/scripts/aglsetup.sh -m raspberrypi5 -b $BUILD_DIR agl-all-features agl-devel agl-ic

# Add meta-qt6 layer
bitbake-layers add-layer $AGL_TOP/$AGL_BRANCH/external/meta-qt6

# Add recipes to local.conf
echo "IMAGE_INSTALL:append = \"$QT_RECIPES\"" >> $LOCAL_CONF
echo "IMAGE_INSTALL:append = \"$DEVEL_RECIPES\"" >> $LOCAL_CONF

# Edit local.conf
echo "SSTATE_DIR = \"$AGL_TOP/sstate-cache/\"" >> $LOCAL_CONF
echo "DL_DIR = \"$HOME/downloads/\"" >> $LOCAL_CONF
echo "INHERIT += \"rm_work\"" >> $LOCAL_CONF
echo "RM_OLD_IMAGE = \"1\"" >> $LOCAL_CONF

# Start creating the image
# time bitbake agl-image-compositor

