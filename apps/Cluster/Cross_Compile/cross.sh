#!/bin/bash
# Script to build, copy, and deploy the Qt cross-compiled app to a Raspberry Pi
# Usage: ./deploy_to_pi.sh <RaspberryPi_IP>

# Check if IP argument is provided
if [ -z "$1" ]; then
    echo "Usage: $0 <RaspberryPi_IP>"
    exit 1
fi

# Configuration
DOCKER_IMAGE="qtcrossbuild"
CONTAINER_NAME="tmpbuild"
LOCAL_BUILD_DIR="$HOME/Cluster/Cross_Compile"
REMOTE_USER="root"
REMOTE_HOST="$1"
REMOTE_DIR="/home/qtApp"

# 1️⃣ Build the Docker image
rsync -av --delete ~/Documents/Team2_TheySEAME_Rollin/apps/Cluster/qtApp/ \
      ~/Documents/Team2_TheySEAME_Rollin/apps/Cluster/Cross_Compile/qtApp/
echo "Building the Docker image..."
sudo docker build -t $DOCKER_IMAGE -f Dockerfile .

# 2️⃣ Remove old container if it exists
if sudo docker ps -a --format '{{.Names}}' | grep -Eq "^${CONTAINER_NAME}\$"; then
    echo "Removing old container..."
    sudo docker rm -f $CONTAINER_NAME
fi

# 3️⃣ Create a new temporary container
echo "Creating temporary container..."
sudo docker create --name $CONTAINER_NAME $DOCKER_IMAGE

# 4️⃣ Copy binary and necessary QML resources from the container
echo "Copying binary and QML resources..."
mkdir -p $LOCAL_BUILD_DIR/qtApp
sudo docker cp $CONTAINER_NAME:/build/project/qtAppExec $LOCAL_BUILD_DIR/qtAppPi/./qtAppExec

# 5️⃣ Copy everything to the Raspberry Pi
echo "Transferring files to Raspberry Pi at $REMOTE_HOST..."
ssh $REMOTE_USER@$REMOTE_HOST "rm -rf $REMOTE_DIR"
scp -r $LOCAL_BUILD_DIR/qtAppPi $REMOTE_USER@$REMOTE_HOST:$REMOTE_DIR
