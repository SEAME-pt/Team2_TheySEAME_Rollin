# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5

## AGL Image

The AGL installation is done through a Dockerfile to ensure the enviroment is correctly set-up for the image generation.
It will generate the same image described in [here](/docs/architecture/AGL)

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t agl .
mkdir agl downloads sstate-cache
docker run -it -v $PWD/agl:/home/buidl/agl -v $PWD/downloads:/home/build/downloads -v $PWD/sstate-cache:home/build/sstate-cache agl
```
