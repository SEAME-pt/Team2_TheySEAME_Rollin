#!/bin/bash
# start_profiling.sh — Launch OpenOCD RTT bridge + SEGGER SystemView
# Usage: ./start_profiling.sh [--no-gui]
#
# Prerequisites:
#   - OpenOCD 0.12+ installed (sudo apt install openocd)
#   - SEGGER SystemView at /opt/SEGGER/SystemView_V362c/SystemView
#   - Firmware built and flashed with SystemView + ThreadX hooks

set -euo pipefail
SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
OPENOCD_CFG="$SCRIPT_DIR/openocd_rtt.cfg"
SYSTEMVIEW="/opt/SEGGER/SystemView_V362c/SystemView"
NO_GUI=false

if [[ "${1:-}" == "--no-gui" ]]; then
    NO_GUI=true
fi

# Check prerequisites
if ! command -v openocd &>/dev/null; then
    echo "ERROR: openocd not found. Install with: sudo apt install openocd"
    exit 1
fi

if [[ ! -f "$OPENOCD_CFG" ]]; then
    echo "ERROR: $OPENOCD_CFG not found"
    exit 1
fi

# Kill any existing OpenOCD
pkill -f "openocd.*openocd_rtt" 2>/dev/null || true
sleep 0.5

echo "Starting OpenOCD RTT bridge..."
openocd -f "$OPENOCD_CFG" &
OPENOCD_PID=$!
echo "OpenOCD PID: $OPENOCD_PID"

# Give OpenOCD time to connect and start RTT
sleep 2

# Verify OpenOCD is still running
if ! kill -0 "$OPENOCD_PID" 2>/dev/null; then
    echo "ERROR: OpenOCD failed to start. Check that the board is connected."
    exit 1
fi

echo ""
echo "╔══════════════════════════════════════════════════════╗"
echo "║  OpenOCD RTT bridge is running                      ║"
echo "║                                                     ║"
echo "║  SystemView data:  TCP localhost:19111 (channel 1)  ║"
echo "║  RTT terminal:     TCP localhost:19112 (channel 0)  ║"
echo "║  OpenOCD telnet:   localhost:4444                   ║"
echo "╚══════════════════════════════════════════════════════╝"
echo ""

if [[ "$NO_GUI" == false ]] && [[ -x "$SYSTEMVIEW" ]]; then
    echo "Launching SystemView..."
    echo "  → In SystemView: Target → Recorder Configuration"
    echo "  → Select TCP/IP, Host: localhost, Port: 19111"
    echo ""
    "$SYSTEMVIEW" &
else
    echo "To connect SystemView manually:"
    echo "  1. Open SystemView"
    echo "  2. Target → Recorder Configuration"
    echo "  3. Select 'TCP/IP'"
    echo "  4. Host: localhost, Port: 19111"
    echo "  5. Click OK, then Target → Start Recording"
fi

echo ""
echo "Press Ctrl+C to stop..."

# Cleanup on exit
cleanup() {
    echo ""
    echo "Stopping OpenOCD..."
    kill "$OPENOCD_PID" 2>/dev/null || true
    wait "$OPENOCD_PID" 2>/dev/null || true
    echo "Done."
}
trap cleanup EXIT INT TERM

# Wait for OpenOCD
wait "$OPENOCD_PID"
