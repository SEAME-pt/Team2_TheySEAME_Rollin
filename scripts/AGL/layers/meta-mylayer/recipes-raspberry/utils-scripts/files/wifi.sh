#!/bin/bash

rfkill unblock wifi
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
udhcpc -i wlan0
echo nameserver 8.8.8.8 >> /etc/resolv.conf
