#!/bin/bash
set -e

if [ which ruby >/dev/null ]; then
    echo "Ruby is already installed"
else
    echo "Installing Ruby..."
    sudo apt-get update
    sudo apt-get install ruby
    sudo apt-get install gcc-arm-none-eabi
fi

if [ which ceedling >/dev/null ]; then
    echo "Ceedling is already installed"
else
    echo "Installing Ceedling..."
    sudo gem install ceedling
fi

ceedling new stm32_firmware_tests
