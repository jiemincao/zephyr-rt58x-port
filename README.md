# Zephyr RTOS Port for Rafael Micro RT58x SoC

This project adds support for the **Rafael Micro RT58x Series (Cortex-M3)** to the Zephyr RTOS.

## 📂 Project Structure

* **`boards/rafael/rt582_evk`**: Board definition, Linker script, Default Kconfig.
* **`soc/rafael/rt58x`**: SoC definition, HAL integration, Startup code.
* **`drivers/serial`**: UART driver implementation (Polling mode supported).
* **`dts/bindings/serial`**: Device Tree Binding for RT58x UART.
* **`install.sh`**: Automated script to setup workspace and patch Zephyr.

## 🚀 How to Use

### 1. Prerequisites
Ensure you have Python 3, `pip`, `cmake`, `dtc`, and `git` installed.
Install the `west` tool:

```bash
pip3 install west
```

### 2. One-Click Installation
Clone this repository and run the install script. This will automatically:
1. Initialize a new Zephyr workspace (zephyr_ws).
2. Download Zephyr source code and dependencies.
3. Apply the RT58x porting files and register them into Zephyr.

```bash
# Clone this repo (replace <your-repo-url> with actual URL)
git clone <your-repo-url> zephyr-rt58x-port
cd zephyr-rt58x-port

# Run installation script
chmod +x install.sh
./install.sh
```
### 3. Build & Flash
After the installation is complete, go to the workspace and build the sample:

```bash
# Enter workspace
cd zephyr_ws
source zephyr/zephyr-env.sh

# Build Hello World
west build -b rt582_evk zephyr/samples/hello_world
```

### Flashing Instructions
Since OpenOCD/J-Link support for RT58x flash programming is not yet integrated upstream, please use the vendor tool:
1. Locate the binary at: build/zephyr/zephyr.bin
2. Use the Rafael Micro ISP Tool to flash the binary.
3. Target Address: 0x00000000 (Flash Start Address).
    Note: This will overwrite the original bootloader. To restore, please re-flash the bootloader.
