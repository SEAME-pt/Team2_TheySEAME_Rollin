#!/bin/bash
set -e

APP_DIR="/usr/bin/qtApp"
BIN_NAME="qtAppRaw"
BIN_EXEC="qtApp"
URL="https://github.com/SEAME-pt/Team2_TheySEAME_Rollin/releases/download/cluster/qtApp"

mkdir -p "$APP_DIR"
cd "$APP_DIR"

echo "Downloading binary ..."
wget "$URL" -O "$BIN_NAME"

if [ ! -f "$BIN_NAME" ]; then
    echo "Download failed"
    exit 1
fi

echo "Updating binary ..."
chmod +x "$BIN_NAME"
mv -f "$BIN_NAME" "$BIN_EXEC"

echo "Update complete"

