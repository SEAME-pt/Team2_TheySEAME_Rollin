# Steps

##### **1 - Creates the Target Image (DockerfileRasp)**

```bash
$ docker buildx build --platform linux/arm64 --load -f Dockerfile-sysroot -t raspimage .
```


##### **2 - Extract the Sysroot Archive**

```bash
$ docker create --name temp-arm raspimage
$ docker cp temp-arm:/build/rasp.tar.gz ./rasp.tar.gz
```
##### **3 - Build the Cross-Compiler and Qt (Dockerfile)**

```bash
$ docker build -t qtcrossbuild -f Dockerfile .
```

##### **4 - Extract the project binaries**

```bash
$ docker create --name tmpbuild qtcrossbuild
$ docker cp tmpbuild:/build/project/appDashboardproject ./qtApp
$ docker cp tmpbuild:/build/qt-pi-binaries.tar.gz ./qt-pi-binaries.tar.gz
```
