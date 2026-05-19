# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5
Useful commands in AGL and some concepts [here](../../docs/AGL/README.md)

## AGL Image

The AGL installation is done through a Dockerfile to ensure the enviroment is correctly set-up for the image generation

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t agl .
mkdir agl downloads sstate-cache -> Only do it the first time, otherwise skip it
docker run -it -v $PWD/agl:/home/build/agl -v $PWD/downloads:/home/build/downloads -v $PWD/sstate-cache:/home/build/sstate-cache agl bash
```

Inside the docker, run

```bash
source aglenv.sh
bitbake <wanted-recipe-name>
bitbake package-index
exit
```

Then go to the **agl/tmp/deploy/rpm** directory and run this command

```bash
python3 -m http.server 8000
```

Inside the RaspberryPi5 (AGL-OS) run this command

```bash
dnf makecache
dnf install <package-name>
```
