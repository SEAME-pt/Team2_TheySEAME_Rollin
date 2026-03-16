#!/bin/bash
# dump_sysview.sh — Dump SystemView post-mortem data from STM32 via OpenOCD
#
# Usage: ./dump_sysview.sh [output_file]
#
# This script:
#   1. Connects to the target via OpenOCD + ST-Link
#   2. Halts the CPU
#   3. Reads the RTT SysView buffer directly from RAM
#   4. Saves it as a .SVDat file for SystemView
#   5. Resumes the target
#
# Prerequisites: firmware built with SEGGER_SYSVIEW_POST_MORTEM_MODE=1

set -euo pipefail

TIMESTAMP="$(date +%Y%m%d_%H%M%S)"
OUTFILE="${1:-sysview_${TIMESTAMP}.SVDat}"
MAP_FILE="Debug/STM32.map"
FIRMWARE_DIR="$(cd "$(dirname "$0")" && pwd)"

cd "$FIRMWARE_DIR"

# Get RTT control block address from map file
RTT_ADDR=$(grep -A1 "\.bss\._SEGGER_RTT" "$MAP_FILE" | grep "0x200" | awk '{print $1}')
if [[ -z "$RTT_ADDR" ]]; then
    echo "ERROR: Cannot find _SEGGER_RTT symbol in $MAP_FILE"
    exit 1
fi
echo "RTT control block: $RTT_ADDR"

# Create OpenOCD dump script
cat > /tmp/ocd_dump.cfg << 'OCDEOF'
source [find interface/stlink.cfg]
source [find target/stm32u5x.cfg]
adapter speed 4000
init
halt

# Read RTT control block to get buffer info
# aUp[1] starts at RTT_BASE + 0x30
# Fields: sName(4), pBuffer(4), SizeOfBuffer(4), WrOff(4), RdOff(4), Flags(4)
OCDEOF

# Append dynamic address
cat >> /tmp/ocd_dump.cfg << OCDEOF

set RTT_BASE $RTT_ADDR
set SYSVIEW_BUF_DESC [expr {\$RTT_BASE + 0x30}]

# Read buffer descriptor for up-channel 1 (SysView)
set buf_ptr   [mrw [expr {\$SYSVIEW_BUF_DESC + 4}]]
set buf_size  [mrw [expr {\$SYSVIEW_BUF_DESC + 8}]]
set wr_off    [mrw [expr {\$SYSVIEW_BUF_DESC + 12}]]
set rd_off    [mrw [expr {\$SYSVIEW_BUF_DESC + 16}]]

echo "SysView buffer: ptr=\$buf_ptr size=\$buf_size WrOff=\$wr_off RdOff=\$rd_off"

# In post-mortem mode, the buffer is circular.
# Dump the entire buffer content and let SystemView sort it out.
if {\$buf_size > 0 && \$wr_off > 0} {
    # Dump the part from RdOff to WrOff (or whole buffer if wrapped)
    if {\$wr_off >= \$rd_off} {
        set dump_size [expr {\$wr_off - \$rd_off}]
        set dump_start [expr {\$buf_ptr + \$rd_off}]
    } else {
        # Wrapped - dump everything
        set dump_size \$buf_size
        set dump_start \$buf_ptr
    }
    echo "Dumping \$dump_size bytes from 0x[format %08x \$dump_start]"
    dump_image $OUTFILE \$dump_start \$dump_size
    echo "Saved to $OUTFILE"
} else {
    echo "ERROR: No data in SysView buffer (WrOff=\$wr_off)"
}

resume
shutdown
OCDEOF

echo "Connecting to target and dumping SystemView data..."
openocd -f /tmp/ocd_dump.cfg 2>&1

if [[ -f "$OUTFILE" ]]; then
    SIZE=$(stat -c%s "$OUTFILE")
    echo ""
    echo "=== Capture complete ==="
    echo "File: $OUTFILE ($SIZE bytes)"
    echo "Open in SystemView: File → Open → select $OUTFILE"
else
    echo ""
    echo "ERROR: No output file created"
    exit 1
fi
