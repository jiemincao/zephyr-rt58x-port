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

# ---------------------------------------------------------
# 1. 環境建置 (Environment Setup)
# ---------------------------------------------------------
if [ -d "$TARGET_DIR" ]; then
    echo "ℹ️  Workspace '$WORKSPACE_NAME' already exists. Skipping init."
else
    echo "📦 Initializing Zephyr workspace..."
    
    # 檢查是否有安裝 west
    if ! command -v west &> /dev/null; then
        echo "❌ Error: 'west' tool is not installed."
        echo "   Please run: pip3 install west"
        exit 1
    fi

    # 執行 west init
    west init "$TARGET_DIR"
    
    # 進入工作區
    cd "$TARGET_DIR"
    
    echo "⬇️  Updating modules (this may take a while)..."
    west update
    
    echo "🐍 Installing Python dependencies..."
    pip3 install -r zephyr/scripts/requirements.txt
    
    echo "⚙️  Exporting Zephyr CMake package..."
    west zephyr-export
    
    # 回到原本目錄準備複製檔案
    cd "$CURRENT_DIR"
fi

# 再次確認 Zephyr Base 是否存在
if [ ! -d "$ZEPHYR_BASE" ]; then
    echo "❌ Error: Zephyr base directory not found at $ZEPHYR_BASE"
    exit 1
fi

echo "✅ Zephyr environment is ready at: $ZEPHYR_BASE"

# ---------------------------------------------------------
# 2. 檔案整合 (File Integration)
# ---------------------------------------------------------
echo "📂 Integrating RT58x Platform files..."

# 複製 Boards
echo "   -> Copying boards/rafael..."
mkdir -p "$ZEPHYR_BASE/boards/rafael"
cp -r boards/rafael/* "$ZEPHYR_BASE/boards/rafael/"

# 複製 SoC (包含 HAL)
echo "   -> Copying soc/rafael..."
mkdir -p "$ZEPHYR_BASE/soc/rafael"
cp -r soc/rafael/* "$ZEPHYR_BASE/soc/rafael/"

# 複製 Driver 檔案
echo "   -> Copying drivers..."
cp drivers/serial/uart_rt58x.c "$ZEPHYR_BASE/drivers/serial/"
cp drivers/serial/Kconfig.rt58x "$ZEPHYR_BASE/drivers/serial/"

# 複製 DTS Binding
echo "   -> Copying DTS bindings..."
cp dts/bindings/serial/rafael,rt58x-uart.yaml "$ZEPHYR_BASE/dts/bindings/serial/"

# ---------------------------------------------------------
# 3. 自動註冊 (Patching Zephyr)
# ---------------------------------------------------------
echo "🔧 Patching Zephyr configuration files..."

# 定義 Patch 函式
append_if_missing() {
    local file="$1"
    local line="$2"
    
    if [ ! -f "$file" ]; then
        echo "   ⚠️ Warning: File $file not found, skipping patch."
        return
    fi

    if ! grep -qF "$line" "$file"; then
        echo "   + Patching $file"
        # 加個換行符號確保不會接在別人後面
        echo "" >> "$file"
        echo "$line" >> "$file"
    else
        echo "   . Skipping $file (already patched)"
    fi
}

# 3.1 註冊 SoC 廠商
append_if_missing "$ZEPHYR_BASE/soc/CMakeLists.txt" 'add_subdirectory(rafael)'

# 3.2 註冊 Board 廠商
append_if_missing "$ZEPHYR_BASE/boards/CMakeLists.txt" 'add_subdirectory(rafael)'

# 3.3 註冊 UART Driver (CMake)
append_if_missing "$ZEPHYR_BASE/drivers/serial/CMakeLists.txt" 'zephyr_library_sources_ifdef(CONFIG_UART_RT58X uart_rt58x.c)'

# 3.4 註冊 UART Driver (Kconfig)
append_if_missing "$ZEPHYR_BASE/drivers/serial/Kconfig" 'source "drivers/serial/Kconfig.rt58x"'

# ---------------------------------------------------------
# 4. 完成與提示
# ---------------------------------------------------------
echo "========================================================"
echo "🎉 Setup Complete!"
echo "========================================================"
echo "To build the sample project, run:"
echo ""
echo "  cd $WORKSPACE_NAME"
echo "  source zephyr/zephyr-env.sh"
echo "  west build -b rt582_evk samples/hello_world"
echo ""