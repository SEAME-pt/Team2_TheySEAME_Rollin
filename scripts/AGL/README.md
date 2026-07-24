# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5
Useful commands in AGL and some concepts [here](../../docs/AGL/README.md)

## Directory Structure
```
├── layers/ # Custom layer
├── conf/ # Scripts and configs
├── Dockerfile # AGL dockerfile
└── README.md # This file
```

The AGL installation is done through a Dockerfile to ensure the enviroment is correctly set-up for the image generation

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t agl .
mkdir -p agl downloads sstate-cache
docker run -it -v $PWD/agl:/home/build/agl -v $PWD/downloads:/home/build/downloads -v $PWD/sstate-cache:/home/build/sstate-cache agl bash
source aglenv.sh
```

## Build AGL image

Inside the docker, run

```bash
bitbake <agl-target-image>
exit
```

Then the image needs to be flashed to a microSD or SSD.
The **flash_sd.sh** flashes the image for a microSD by default.
For a SSD the same script works if the device in the script is edited (/dev/sdb instead of /dev/sda, depending on the system) to the SSD one

Before using this script, it is recomended to verify the correct device name using the lstblk to avoid confusion

```bash
bash flash_sd.sh
```

After this command finishes, put the microSD or plug the SSD and turn on the RPI5

## Build Recipe

This only applies if you already have an AGL image done and running in the RPI5.
Also, if you don't have the rpm_server set-up done, you can follow [this](../../docs/AGL/HttpRPM.md)

Inside the docker, run

```bash
bitbake <wanted-recipe-name>
bitbake package-index
exit
```

Then run this script to be able to install rpm packages inside the RPI5

```bash
bash rpm_server.sh
```

Inside the RPI5 (AGL-OS) run this command

```bash
dnf makecache
dnf install <package-name>
```
