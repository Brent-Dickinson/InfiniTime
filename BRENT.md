# PineTime / InfiniTime Custom Firmware Workflow

## Hardware / Machines

- **Windows laptop** – editing only (VS Code)
- **Lenovo Ubuntu 22.04** – canonical build machine (Docker)
- **Samsung S22 (Termux + Gadgetbridge)** – DFU flashing over BLE

---

# Core Principle

Edit anywhere.  
Build only on the Lenovo.  
Always build inside Docker.  
Never run host `cmake` on the Lenovo.

---

# Normal Workflow

## 1. Edit Code

- Edit in VS Code (Windows or Lenovo).
- Commit and push to your fork.

---

## 2. Build on Lenovo (Docker Only)

SSH into Lenovo:

```bash
cd ~/InfiniTime
git pull
```

If you previously ran host CMake, clean it once:

```bash
rm -rf build
```

### Build (canonical command)

```bash
docker build -t infinitime-builder -f docker/Dockerfile docker

docker run --rm -it \
  -v "$PWD":/sources \
  infinitime-builder \
  /sources/docker/build.sh
```

Output will appear in:

```bash
build/output/
```

You should see:

```bash
pinetime-mcuboot-app-dfu-<version>.zip
```

---

## 3. Send Firmware to Phone

Find phone IP (Tailscale or local network).

```bash
scp -P 8022 build/output/pinetime-mcuboot-app-dfu-<version>.zip <phone_ip>:~
```

---

## 4. Flash via Gadgetbridge

On phone:

- Open Gadgetbridge
- Connect PineTime
- Install firmware
- Select the DFU ZIP

---

# Setup (One Time Only)

## Clone Your Fork

```bash
git clone <your fork URL>
cd InfiniTime
git submodule update --init --recursive
```

---

## Install Docker (Lenovo)

```bash
sudo apt update
sudo apt install docker.io
sudo usermod -aG docker $USER
```

Log out and back in after adding yourself to the docker group.

---

## Install Gadgetbridge (Phone)

1. Install F-Droid
2. Install Gadgetbridge
3. Grant Bluetooth permissions

---

# Important Notes

### Do NOT:

- Run host `cmake` on Lenovo.
- Mix host builds and Docker builds in the same `build/` directory.
- Install the NRF5 SDK manually on the host unless intentionally doing host-native builds.

### If CMake complains about path mismatch:

```bash
rm -rf build
```

Then rebuild via Docker.

---

# Mental Model

Lenovo = build server  
Docker = toolchain + SDK  
Windows = editor only  
Phone = DFU target
