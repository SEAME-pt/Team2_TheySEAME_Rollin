# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5

## AGL Image

The AGL installation is done through a Dockerfile to ensure the enviroment is correctly set-up for the image generation.
It will generate the same image described in [here](https://github.com/SEAME-pt/Team2_TheySEAME_Rollin/blob/develop/docs/architecture/AGL/README.md)

```bash
docker build -t agl <path-to-Dockerfile>
docker run -it -v $PWD/build:/home/buidl/agl agl
... inside docker container ...
bash AGL_installation.sh
```
