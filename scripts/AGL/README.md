# AGL Scripts

Here is some scripts that set-up AGL in the Raspberry Pi 5

## Directory Structure
```
├── layers/ # Custom layer
├── conf/ # Scripts and configs
├── Dockerfile # AGL dockerfile
└── README.md # This file
```

## Run the script

```bash
docker build --build-arg UID=$(id -u) --build-arg GID=$(id -g) -t agl .
mkdir -p agl downloads sstate-cache
docker run -it -v $PWD/agl:/home/build/agl -v $PWD/downloads:/home/build/downloads -v $PWD/sstate-cache:/home/build/sstate-cache agl bash
```
