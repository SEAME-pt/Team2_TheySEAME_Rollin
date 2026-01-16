# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5

## AGL Image

The AGL installation is done through a Dockerfile to ensure the enviroment is correctly set-up for the image generation.
It will generate the same image described in [here](/docs/architecture/AGL)

```bash
docker build -t agl .
mkdir agl
docker run -it -u $(id -u):$(id -g) -v $PWD/agl:/home/buidl/agl agl
```
