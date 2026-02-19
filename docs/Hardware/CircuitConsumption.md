# Circuit Consumption

This is done in DC

Glossary:
- RN: Running
- ST: Stall
- MAX: Maximum
- CP: Components

## Components

- [RPI5](#RPI5)
- [Microcontroller](#Microcontroller)
- [Motors](#Motors)
- [TouchScreen](#TouchScreen)

## RPI5 Components

All components

$P_{RN\_RPI5\_CP} = 7,5 + 6,6 + 1,65 = 15,75W$  
$P_{MAX\_RPI5\_CP} = 25 + 6,6 + 3,3 = 34,9W$  

### RPI5

$V = 5V$  
$I_{RN} = 1,5A$  
$I_{MAX} = 5A$  
$P_{RN} = 5 \cdot 1,5 = 7,5W$  
$P_{MAX} = 5 \cdot 5 = 25W$  

### RPI5-SSD

$V = 3.3V$  
$I = 2.0A$  
$P = 3.3 \cdot 2.0 = 6,6W$  

### RPI5-Hailo 26-Tops

$V = 3,3V$  
$I_{RN} = 500mA$  
$I_{MAX} = 1A$  
$P_{RN} = 3,3 \cdot 0,5 = 1,65W$  
$P_{MAX} = 3,3 \cdot 1 = 3,3W$  

## Microcontroller

$V = 5V$  
$I = 500mA$  
$P = 5 \cdot 0,5 = 2,5W$  

## Motors

All components

$P_{RN\_Motors} = 1,44 + 2,5 = 3,94W$  
$P_{ST\_Motors} = 14,4 + 6 = 20,4W$  

### Back Motors

$V = 6~15V$  
$I_{RN} = 120mA (12V)$  
$I_{ST} = 1.2A (12V)$  
$P_{RN} = 12 \cdot 0,12 = 1,44W$  
$P_{ST} = 12 \cdot 1,2 = 14,4W$  

### Servo Motor

$V = 4.8V~7.2V$  
$I_{RN} = 500mA$  
$I_{ST} = 2.5A at (6V)$  
$P_{RN} = 5V \cdot 0,500 = 2,5W$  
$P_{ST} = 5V \cdot 1,2 = 6W$  

## TouchScreen

$V = 5V$  
$I = 0,6A$  
$P = 5 \cdot 0,6 = 3W$  

## Other Chips

All other chips not listed here have a negligible current consumption to our calculations

# Circuit Current

$P_{RN} = P_{RN\_RPI5\_CP} + P_{Micro} + P_{RN\_Motors} + P_{Touchscreen}$  
$P_{RN} = 15,75 + 2,5 + 3,94 + 3 = 25,19W$  
$P_{MAX} = P_{MAX\_PI5\_CP} + P_{Micro} + P_{ST\_Motors} + P_{TouchScreen}$  
$P_{MAX} = 34,9 + 2,5 + 20,4 + 3 = 60,8W$  

