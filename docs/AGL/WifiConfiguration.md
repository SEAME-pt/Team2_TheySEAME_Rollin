# Wifi Configuration in AGL

This document describes how to configure a wifi network in AGL.

This project only care about the 1 rule, because the **wifi.service** takes care of the rest

## Steps

1. Setup the network in the wpa config

This is only done one time per network

```bash
wpa_passphrase <SSID> <passphrase> | tee /etc/wpa_supplicant.conf
```

2. Unblock the wifi in rfkill

If the rfkill is blocking the wifi, you need to unblock it

```bash
rfkill unblock wifi
```

3. Connect to the network

```bash
wpa_supplicant -B -i wlan0 -c /etc/wpa_supplicant.conf
```

4. Get an Ip address

You can use **udhcpc** or **dhcpcd**

```bash
udhcpc -i wlan0
```

5. Setup dns server

```bash
echo nameserver 8.8.8.8 >> /etc/resolv.conf
```

