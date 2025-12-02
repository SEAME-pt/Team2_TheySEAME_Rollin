# Steps

##### **1 - Creates the Target Image (Dockerfile-sysroot)**

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
##### **5 - Deploy for the Raspberry Pi**
After the extract on host:
```bash
$ scp qtApp qt-pi-binaries.tar.gz pi@<RPI_IP_ADDRESS>:/home/
```

Then on the Raspberry Pi:
```bash
$ sudo mkdir -p /usr/local/qt6
$ sudo tar -xvf qt-pi-binaries.tar.gz -C /usr/local/qt6

$ ./qtApp
```

###  Deployment Script

To simplify testing and deployment on the Raspberry Pi, we use a Bash script `deploy_to_pi.sh`. This script automates building the Docker image, extracting the compiled binary, and copying it to the target device.

**Key points of the script:**
- Accepts the Raspberry Pi IP as an argument.
- Builds the Docker image (`qtcrossbuild`) containing the cross-compiled application.
- Removes any old temporary container (`tmpbuild`) and creates a new one.
- Copies the compiled binary from the container to a local directory.
- Transfers the files to the Raspberry Pi using `scp`.

**Usage:**
```bash
./deploy_to_pi.sh <RaspberryPi_IP>
```

This script ensures that after any changes in the project, you can quickly rebuild and deploy the updated ARM64 application without manually copying files, streamlining the cross-compilation workflow and reducing the chance of errors.
