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
```

Inside the docker, run

```bash
source aglenv.sh
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
