# Process

## A. Clone InfiniTime (only done once - I forked a branch to my github)

	```
	git clone https://github.com/InfiniTimeOrg/InfiniTime.git
	git submodule status
	git submodule update --init --recursive
	```

---

## B. Install Docker (only done once)

	```bash
	sudo apt update
	sudo apt install docker.io
	```

---

## C. Build the firmware

**Important:** remove any previous host-side build directory first.

```bash
cd ~/embedded/InfiniTime
rm -rf build
```

Run the container build:

```bash
sudo docker run --rm -v $(pwd):/sources infinitime-build
```

This produces firmware artifacts in:

```
build/output/
```

---

## E. Identify the correct DFU package

Use **only** the application DFU zip:

```bash
ls build/output
```

The file you want looks like:

```
pinetime-mcuboot-app-dfu-<version>.zip
```

Do **not** use:

* recovery zips
* resource zips
* raw `.bin` files

---

## F. Transfer DFU zip to Android phone (via SCP)

1. On the laptop, from within /build/output directory:
```
scp -P 8022 pinetime-mcuboot-app-dfu-1.16.0.zip 192.168.1.144:~
```
2. Verify on phone (Termux):
```
ls
```

---

## G. Install Gadgetbridge (only done once)

On the phone:

1. Install **F-Droid** from [https://f-droid.org](https://f-droid.org)
2. Install **Gadgetbridge** from F-Droid
3. Grant Bluetooth / Nearby Devices permissions

---

## H. Flash firmware to PineTime (BLE DFU)

1. Open **Gadgetbridge**
2. Ensure PineTime is paired and connected
3. Menu → **File Installer**
4. Select pinetime-mcuboot-app-dfu-<version>.zip (should be the only thing available)

---