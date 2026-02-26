#!/bin/bash
#
# Fix SEGGER assembly build after STM32CubeIDE regenerates makefiles
# Run this script after IDE regenerates Debug/Middlewares/SEGGER/RTT/subdir.mk
#

MAKEFILE="Debug/Middlewares/SEGGER/RTT/subdir.mk"

if [ ! -f "$MAKEFILE" ]; then
    echo "Error: $MAKEFILE not found"
    exit 1
fi

# Check if SEGGER includes are already present
if grep -q "I../Middlewares/SEGGER/RTT.*I../Middlewares/SEGGER/SystemView.*I../Middlewares/SEGGER/Config" "$MAKEFILE"; then
    echo "✓ SEGGER include paths already present"
    exit 0
fi

# Add SEGGER include paths to assembly compilation rule
sed -i 's|\(-c -I../Core/Inc\)|\1 -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config|' "$MAKEFILE"

if grep -q "I../Middlewares/SEGGER/RTT.*I../Middlewares/SEGGER/SystemView.*I../Middlewares/SEGGER/Config" "$MAKEFILE"; then
    echo "✓ SEGGER include paths added successfully"
    exit 0
else
    echo "✗ Failed to add SEGGER include paths"
    exit 1
fi
