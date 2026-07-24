# Deps Script

This script installs the dependecies needed in the Host PC to compile code under the rpi5-firmware directory

These are the deps:
- Protobuf v4.25.8
- gRPC v1.60.1

Any previous installation of these 2 packages could make conflicts whenever using this script.
To make this script will work, try to uninstall (if installed on your system) these packages if they have other versions than
the mentioned ones

This versions are used by AGL at the time we used it so, if you are using a newer AGL version or using a newer meta-oe layer version
you will need to update this packages versions to match the AGL ones
