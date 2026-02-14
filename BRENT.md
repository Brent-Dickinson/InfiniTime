# PineTime/InfiniTime tweaking

* Laptop: Ubuntu 22.04 with VS Code
* Phone: Samsung S22 with Termux
* InfiniTime firmware
* Docker-based build
* BLE DFU via phone

---

The approach is:

* Edit personal fork of InfiniTime with VS code, run test build in dev container.
* Use **Docker** to encapsulate the ARM toolchain and NRF5 SDK exactly as InfiniTime expects.
* Build firmware locally into **DFU ZIP artifacts** suitable for MCUBoot.
* Transfer the DFU ZIP to an Android phone.
* Flash the firmware to PineTime **over BLE** using **Gadgetbridge** (via F-Droid).

---

## Normal workflow

1. When editing code in VS Code, make sure to open in dev container
2. After saving changes to code, run a test build in VS Code (takes a very long time):

```
# if needed remove existing build directory:
rm -rf build
cmake -S . -B build -G "Unix Makefiles" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_TOOLCHAIN_FILE=cmake-nRF5x/arm-gcc-toolchain.cmake \
  -DARM_NONE_EABI_TOOLCHAIN_PATH=/opt/gcc-arm-none-eabi-10.3-2021.10 \
  -DNRF5_SDK_PATH=/opt/nRF5_SDK_15.3.0_59ac345
cmake --build build -j
```

3. With no build errors, go to Linux terminal and run docker build:

```
sudo docker run --rm -v $(pwd):/sources infinitime-build
```

4. Send to phone:
* open Termux on phone, run command 
```
sshd
```
* then at laptop, run:

```
scp -P 8022 build/output/pinetime-mcuboot-app-dfu-<version>.zip <phone_ip>:~
```

# Setup - only done once

## Clone InfiniTime

```
git clone https://github.com/InfiniTimeOrg/InfiniTime.git
# go to github, create fork, verify in my github. 
# remove infitime repo, clone my fork from my github.
# may need to update submodules.
git submodule status
git submodule update --init --recursive
```

---

## Install Docker

```bash
sudo apt update
sudo apt install docker.io
```

---

## Install Gadgetbridge

On the phone:

1. Install **F-Droid** from [https://f-droid.org](https://f-droid.org)
2. Install **Gadgetbridge** from F-Droid
3. Grant Bluetooth / Nearby Devices permissions

---