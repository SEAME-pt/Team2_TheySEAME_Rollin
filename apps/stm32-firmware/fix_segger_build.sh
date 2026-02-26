#!/bin/bash
#
# Fix SEGGER assembly build after STM32CubeIDE regenerates makefiles
# Run this script after IDE regenerates Debug/Middlewares/SEGGER/RTT/subdir.mk
#
# The IDE regenerates the .S assembly rule WITHOUT SEGGER include paths,
# while the .c rule keeps them. This script patches the assembly rule only.
#

MAKEFILE="Debug/Middlewares/SEGGER/RTT/subdir.mk"

if [ ! -f "$MAKEFILE" ]; then
    echo "Error: $MAKEFILE not found (run from apps/stm32-firmware/)"
    exit 1
fi

# Check if the assembly rule (.S line + its gcc command on next line) has SEGGER includes
# We look at the gcc line that follows the %.S pattern line  
if awk '/%.S/{getline; if(/SEGGER/) print "found"}' "$MAKEFILE" | grep -q "found"; then
    echo "✓ SEGGER include paths already present in assembly rule"
    exit 0
fi

# Add SEGGER include paths to the assembly (.S) gcc command line only
# Match: line after %.S pattern that contains "assembler-with-cpp" but NOT "SEGGER"
sed -i '/%.S/,/assembler-with-cpp/{/assembler-with-cpp/s|-c -I../Core/Inc|-c -I../Core/Inc -I../Middlewares/SEGGER/RTT -I../Middlewares/SEGGER/SystemView -I../Middlewares/SEGGER/Config|}' "$MAKEFILE"

if awk '/%.S/{getline; if(/SEGGER/) print "found"}' "$MAKEFILE" | grep -q "found"; then
    echo "✓ SEGGER include paths added to assembly rule"
    exit 0
else
    echo "✗ Failed to add SEGGER include paths"
    exit 1
fi
