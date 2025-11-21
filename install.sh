#!/bin/bash

# 設定目標工作區名稱
WORKSPACE_NAME="zephyr_ws"
CURRENT_DIR=$(pwd)
TARGET_DIR="$CURRENT_DIR/$WORKSPACE_NAME"
ZEPHYR_BASE="$TARGET_DIR/zephyr"

set -e # 遇到錯誤立即停止

echo "========================================================"
echo "🚀 Starting RT58x Zephyr Environment Setup"
echo "========================================================"

# 1. 環境建置 (略，與之前相同，如果已存在會跳過)
if [ ! -d "$ZEPHYR_BASE" ]; then
    echo "📦 Initializing Zephyr workspace..."
    if ! command -v west &> /dev/null; then
        echo "❌ Error: 'west' tool not installed."
        exit 1
    fi
    west init "$TARGET_DIR"
    cd "$TARGET_DIR"
    west update
    pip3 install -r zephyr/scripts/requirements.txt
    west zephyr-export
    cd "$CURRENT_DIR"
fi

echo "✅ Zephyr found at: $ZEPHYR_BASE"

# 2. 檔案整合
echo "📂 Integrating RT58x Platform files..."

# 2.1 複製 Boards (包含 index.rst)
# 目標: zephyr/boards/rafael/
mkdir -p "$ZEPHYR_BASE/boards/rafael"
cp -r boards/rafael/* "$ZEPHYR_BASE/boards/rafael/"
echo "   - Copied boards/rafael"

# 2.2 複製 SoC (包含 rt58x 系列和中間層檔案)
# 目標: zephyr/soc/rafael/
mkdir -p "$ZEPHYR_BASE/soc/rafael"
cp -r soc/rafael/* "$ZEPHYR_BASE/soc/rafael/"
echo "   - Copied soc/rafael"

# 2.3 複製 DTS (SoC 定義)
# 目標: zephyr/dts/arm/rafael/rt582.dtsi
mkdir -p "$ZEPHYR_BASE/dts/arm/rafael"
cp dts/arm/rafael/rt582.dtsi "$ZEPHYR_BASE/dts/arm/rafael/"
echo "   - Copied dts/arm/rafael/rt582.dtsi"

# 2.4 複製 DTS Bindings
cp dts/bindings/serial/rafael,rt58x-uart.yaml "$ZEPHYR_BASE/dts/bindings/serial/"
echo "   - Copied DTS binding"

# 2.5 複製 Driver
cp drivers/serial/uart_rt58x.c "$ZEPHYR_BASE/drivers/serial/"
cp drivers/serial/Kconfig.rt58x "$ZEPHYR_BASE/drivers/serial/"
echo "   - Copied UART driver"

# 3. 自動註冊 (Patching)
echo "🔧 Patching Zephyr configuration files..."

append_if_missing() {
    local file="$1"
    local line="$2"
    if [ ! -f "$file" ]; then echo "⚠️ File $file not found"; return; fi
    if ! grep -qF "$line" "$file"; then
        echo "   + Patching $file"
        echo -e "\n$line" >> "$file"
    fi
}

# 3.1 註冊 Vendor Prefix (關鍵！)
append_if_missing "$ZEPHYR_BASE/dts/bindings/vendor-prefixes.txt" "rafael	Rafael Microelectronics"

# 3.2 註冊 UART Driver
append_if_missing "$ZEPHYR_BASE/drivers/serial/CMakeLists.txt" 'zephyr_library_sources_ifdef(CONFIG_UART_RT58X uart_rt58x.c)'
append_if_missing "$ZEPHYR_BASE/drivers/serial/Kconfig" 'source "drivers/serial/Kconfig.rt58x"'

echo "========================================================"
echo "🎉 Setup Complete!"
echo "   Run: west build -b rt582_evk samples/hello_world"
echo "========================================================"