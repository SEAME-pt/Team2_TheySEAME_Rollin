# CAN Communication for STM32 - Detailed Guide (MCP2515/HW-184)

## Table of Contents
1. [Overview](#overview)
2. [Hardware Requirements](#hardware-requirements)
3. [Software Setup](#software-setup)
4. [How CAN Works](#how-can-works)
5. [Function Reference](#function-reference)
6. [Integration Guide](#integration-guide)
7. [Raspberry Pi Integration](#raspberry-pi-integration)
8. [Troubleshooting](#troubleshooting)

---

## Overview

This CAN communication module allows the STM32U585 microcontroller to send battery percentage data to a Raspberry Pi over a CAN bus. The implementation uses **MCP2515 CAN controllers (HW-184 modules)** with software SPI for compatibility.

### What Does This Code Do?

- **Initializes** the MCP2515 CAN controller via software SPI
- **Sends** battery percentage (0-100%) as a single byte over CAN
- **Uses** CAN ID `0x101` for battery messages
- **Compatible** with standard CAN (500 kbit/s default)

### Files Included

```
Core/
├── Inc/
│   └── mcp2515.h           # MCP2515 driver header
└── Src/
    ├── mcp2515.c           # MCP2515 driver implementation
    └── main.c              # Integration in battery thread
```

---

## Hardware Requirements

### STM32 Side

1. **Microcontroller**: STM32U585AIIxQ (B-U585I-IOT02A board)
2. **CAN Module**: HW-184 (MCP2515 + TJA1050 transceiver)
3. **Connections**:
   
   **Power from CN3 (STMod+ connector)**:
   ```
   HW-184 Pin    →    CN3 Pin Number    →    Signal
   ─────────────────────────────────────────────────
   VCC           →    Pin 6              →    5V
   GND           →    Pin 16             →    GND
   ```
   
   **SPI Signals from CN14 (Arduino connector)**:
   ```
   HW-184 Pin    →    CN14 Pin    →    STM32 Pin    →    Function
   ───────────────────────────────────────────────────────────────
   CS            →    Pin 3       →    PD15         →    Chip Select
   SCK           →    Pin 8       →    PF13         →    SPI Clock
   SO (MISO)     →    Pin 1       →    PD9          →    Master In
   SI (MOSI)     →    Pin 2       →    PD8          →    Master Out
   INT           →    Pin 5       →    PE7          →    Interrupt (optional)
   ```

### Raspberry Pi Side

1. **CAN Module**: HW-184 (MCP2515 + TJA1050 transceiver)
2. **Connections**: Connect to Raspberry Pi SPI pins (see RPi documentation)
3. **Termination**: 120Ω resistor between CANH and CANL (at both ends of bus)

### Physical Wiring

```
STM32 Board              CAN Bus              Raspberry Pi
    |                       |                      |
[HW-184 Module] ---CANH---[====]---CANH--- [HW-184 Module]
    |            \         |         /            |
    |             \        |        /             |
[HW-184 Module] ---CANL---[====]---CANL--- [HW-184 Module]
    |                       |                      |
   GND <------------------->|<------------------> GND
   
   Add 120Ω resistor between CANH-CANL on EACH module!
```

**Important**: CAN bus requires 120Ω termination resistors at **both ends** of the bus!

---

## Software Setup

### Step 1: Configure GPIO in STM32CubeMX

Open your `.ioc` file and configure the following pins:

**PD15 (Chip Select)**
- Mode: **GPIO_Output**
- User Label: `MCP2515_CS`
- Configuration:
  - GPIO output level: **High**
  - GPIO mode: **Output Push Pull**
  - GPIO Pull-up/Pull-down: **No pull-up and no pull-down**
  - Maximum output speed: **High**

**PF13 (SCK - Clock)**
- Mode: **GPIO_Output**
- User Label: `MCP2515_SCK`
- Configuration:
  - GPIO output level: **Low**
  - GPIO mode: **Output Push Pull**
  - Maximum output speed: **High**

**PD8 (MOSI - Master Out)**
- Mode: **GPIO_Output**
- User Label: `MCP2515_MOSI`
- Configuration:
  - GPIO output level: **Low**
  - GPIO mode: **Output Push Pull**
  - Maximum output speed: **High**

**PD9 (MISO - Master In)**
- Mode: **GPIO_Input**
- User Label: `MCP2515_MISO`
- Configuration:
  - GPIO mode: **Input mode**
  - GPIO Pull-up/Pull-down: **Pull-up**

**PE7 (Interrupt - Optional)**
- Mode: **GPIO_EXTI7**
- User Label: `MCP2515_INT`
- Configuration:
  - GPIO mode: **External Interrupt Mode with Falling edge trigger**
  - GPIO Pull-up/Pull-down: **Pull-up**

### Step 2: Enable EXTI7 Interrupt (Optional)

- Go to **NVIC** tab
- Find **EXTI line[9:5] interrupts**
- Check **Enabled**

### Step 3: Generate Code

- Click **Project → Generate Code**
- Or press **Ctrl+Shift+G**

### Step 4: Verify Generated Definitions

In `main.h`, you should see:
```c
#define MCP2515_CS_Pin GPIO_PIN_15
#define MCP2515_CS_GPIO_Port GPIOD
#define MCP2515_SCK_Pin GPIO_PIN_13
#define MCP2515_SCK_GPIO_Port GPIOF
#define MCP2515_MOSI_Pin GPIO_PIN_8
#define MCP2515_MOSI_GPIO_Port GPIOD
#define MCP2515_MISO_Pin GPIO_PIN_9
#define MCP2515_MISO_GPIO_Port GPIOD
#define MCP2515_INT_Pin GPIO_PIN_7
#define MCP2515_INT_GPIO_Port GPIOE
```

### Step 5: Add Driver Files

The driver files are already created:
- `Core/Inc/mcp2515.h`
- `Core/Src/mcp2515.c`

They will be automatically compiled with your project.

---

## How CAN Works

### CAN Frame Structure

A CAN message (frame) consists of:

```
┌─────────────┬──────────────┬────────────────┬─────────┐
│ START BIT   │ IDENTIFIER   │ DATA (0-8 B)   │ CRC/ACK │
└─────────────┴──────────────┴────────────────┴─────────┘
     1 bit      11 or 29 bits   0-64 bits      15 bits
```

**Our Implementation Uses**:
- **Standard 11-bit Identifier**: `0x101` (257 in decimal)
- **Data Length**: 1 byte
- **Data**: Battery percentage (0-100)

### Message Flow

```
STM32                         CAN Bus                    Raspberry Pi
  |                              |                             |
  | 1. Calculate battery %       |                             |
  | 2. Call CAN_SendBattery()    |                             |
  | 3. Build CAN frame           |                             |
  |----------------------------->|                             |
  |   [ID:0x101][Data:85]        |                             |
  |                              |<----------------------------|
  |                              | 4. Receives frame           |
  |                              | 5. Extracts data            |
  |                              | 6. Displays: "Battery: 85%" |
```

### CAN Bus Priority

CAN uses identifier-based priority. **Lower ID = Higher Priority**

Example priority order (highest to lowest):
- `0x001` - Emergency stop
- `0x010` - Critical sensor data
- `0x101` - Battery data (our message)
- `0x200` - Status updates

---

## Function Reference

### `MCP2515_Init()`

**Purpose**: Initializes the MCP2515 CAN controller and sets bus speed

**Prototype**:
```c
HAL_StatusTypeDef MCP2515_Init(MCP2515_Speed_t speed);
```

**Parameters**:
- `speed`: CAN bus speed
  - `CAN_SPEED_500KBPS` - 500 kbit/s (recommended)
  - `CAN_SPEED_250KBPS` - 250 kbit/s
  - `CAN_SPEED_125KBPS` - 125 kbit/s
  - `CAN_SPEED_100KBPS` - 100 kbit/s

**Returns**:
- `HAL_OK`: Initialization successful
- `HAL_ERROR`: Initialization failed

**Usage Example**:
```c
#include "mcp2515.h"

void setup_can(void) {
    if (MCP2515_Init(CAN_SPEED_500KBPS) == HAL_OK) {
        Debug_Print("CAN initialized successfully\r\n");
    } else {
        Debug_Print("CAN initialization FAILED\r\n");
    }
}
```

**What It Does Internally**:
1. Resets the MCP2515 chip via SPI
2. Sets configuration mode
3. Configures bit timing for selected speed (assumes 8MHz crystal)
4. Disables interrupts (polling mode)
5. Sets normal operating mode

**When to Call**:
- Once during system initialization
- After GPIO pins have been configured
- Before sending any CAN messages

---

### `MCP2515_SendBattery()`

**Purpose**: Sends battery percentage over CAN bus

**Prototype**:
```c
HAL_StatusTypeDef MCP2515_SendBattery(uint8_t percentage);
```

**Parameters**:
- `percentage`: Battery percentage (0-100)

**Returns**:
- `HAL_OK`: Message sent successfully
- `HAL_BUSY`: TX buffer busy
- `HAL_TIMEOUT`: Transmission timeout

**Usage Example**:
```c
uint8_t battery_level = 85;
HAL_StatusTypeDef status = MCP2515_SendBattery(battery_level);

if (status == HAL_OK) {
    Debug_Print("Battery sent via CAN\r\n");
} else if (status == HAL_BUSY) {
    Debug_Print("CAN TX buffer busy\r\n");
} else {
    Debug_Print("CAN send failed\r\n");
}
```

**What It Does Internally**:

1. **Checks TX Buffer**:
   - Verifies TX buffer 0 is free
   
2. **Configures Message**:
   ```c
   CAN ID: 0x101 (standard 11-bit)
   Data Length: 1 byte
   Data: [percentage]
   ```

3. **Sends Message**:
   - Writes to TX buffer registers
   - Triggers transmission
   - Waits for completion (100ms timeout)

**CAN Frame Structure**:
```
┌──────────┬─────────────┬──────────────┬─────────┐
│ SOF      │ ID: 0x101   │ DLC: 1       │ DATA    │
│ (1 bit)  │ (11 bits)   │ (4 bits)     │ (1 byte)│
└──────────┴─────────────┴──────────────┴─────────┘
                                         └──> Battery %
```

**Timing Considerations**:
- Message transmission: ~200 µs at 500 kbit/s
- Timeout: 100ms maximum wait
- Recommended interval: ≥100 ms between messages

---

## Integration Guide

### Already Integrated in Your Code!

The CAN communication is **already working** in your battery monitoring thread. Here's what happens:

**In `Battery_Thread_Entry()` function**:

```c
void Battery_Thread_Entry(ULONG thread_input) {
    // ... initialization code ...
    
    // Initialize MCP2515 CAN controller
    Debug_Print("\r\n[CAN] Initializing MCP2515...\r\n");
    if (MCP2515_Init(CAN_SPEED_500KBPS) == HAL_OK) {
        Debug_Print("[CAN] MCP2515 initialized successfully (500 kbps)\r\n");
    } else {
        Debug_Print("[CAN] MCP2515 initialization FAILED!\r\n");
    }
    
    // ... INA219 setup and motor tests ...
    
    while(1) {
        // Read battery voltage and calculate percentage
        // ... battery reading code ...
        
        uint8_t percentage_int = (uint8_t)percentage;
        
        // Send battery percentage over CAN
        HAL_StatusTypeDef can_status = MCP2515_SendBattery(percentage_int);
        const char* can_status_str = (can_status == HAL_OK) ? "OK" : 
                                      (can_status == HAL_BUSY) ? "BUSY" : 
                                      (can_status == HAL_TIMEOUT) ? "TIMEOUT" : "ERROR";
        
        // Print with CAN status
        snprintf(uart_buf, sizeof(uart_buf), 
                "[BATTERY] %d.%02dV (%d%%), %dmA | CAN: %s 🤑\r\n",
                voltage_int, voltage_frac, percentage_int, current_int, can_status_str);
        Debug_Print(uart_buf);
        
        tx_thread_sleep(200);  // 2 seconds
    }
}
```

### What You'll See on Serial Output

```
[CAN] Initializing MCP2515...
[CAN] MCP2515 initialized successfully (500 kbps)
[BATTERY_THREAD] Starting monitoring loop

[BATTERY] 12.45V (93%), 125mA | CAN: OK 🤑
[BATTERY] 12.43V (92%), 120mA | CAN: OK 🤑
[BATTERY] 12.41V (91%), 118mA | CAN: OK 🤑
```

### Customization Options

**Change CAN Bus Speed**:
```c
// In Battery_Thread_Entry(), change:
MCP2515_Init(CAN_SPEED_250KBPS);  // Use 250 kbps instead
```

**Change Send Interval**:
```c
// At end of loop:
tx_thread_sleep(100);  // Send every 1 second instead of 2
```

**Change CAN ID**:
```c
// Edit mcp2515.c, function MCP2515_SendBattery():
uint16_t can_id = 0x102;  // Use different ID
```

---

## Raspberry Pi Integration

### Python Example (using python-can)

**Installation**:
```bash
sudo apt-get install can-utils
sudo pip3 install python-can
```

**Enable CAN Interface**:
```bash
sudo ip link set can0 type can bitrate 500000
sudo ip link set can0 up
```

**Python Receiver Code**:
```python
import can
import time

def receive_battery_data():
    # Create CAN bus instance
    try:
        bus = can.interface.Bus(channel='can0', 
                                 bustype='socketcan', 
                                 bitrate=500000)
        print("CAN bus initialized successfully")
    except Exception as e:
        print(f"Error initializing CAN: {e}")
        return
    
    print("Waiting for battery messages...")
    
    while True:
        try:
            # Receive message with 1 second timeout
            msg = bus.recv(timeout=1.0)
            
            if msg is None:
                continue
            
            # Check if it's a battery message
            if msg.arbitration_id == 0x101:
                # Extract battery percentage (first byte)
                battery_percentage = msg.data[0]
                
                # Print with timestamp
                timestamp = time.strftime("%H:%M:%S")
                print(f"[{timestamp}] Battery: {battery_percentage}%")
                
                # Optional: Check battery level
                if battery_percentage < 20:
                    print("⚠️  WARNING: Low battery!")
                elif battery_percentage < 50:
                    print("ℹ️  Battery moderate")
                else:
                    print("✅ Battery good")
        
        except KeyboardInterrupt:
            print("\nStopping receiver...")
            break
        except Exception as e:
            print(f"Error receiving: {e}")
    
    # Cleanup
    bus.shutdown()

if __name__ == "__main__":
    receive_battery_data()
```

**Running the Code**:
```bash
python3 battery_receiver.py
```

**Expected Output**:
```
CAN bus initialized successfully
Waiting for battery messages...
[15:30:45] Battery: 87%
✅ Battery good
[15:30:47] Battery: 86%
✅ Battery good
[15:30:49] Battery: 85%
✅ Battery good
```

### C++ Receiver (for performance-critical applications)

```cpp
#include <iostream>
#include <linux/can.h>
#include <linux/can/raw.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <unistd.h>
#include <cstring>

int main() {
    int sock = socket(PF_CAN, SOCK_RAW, CAN_RAW);
    struct ifreq ifr;
    std::strcpy(ifr.ifr_name, "can0");
    ioctl(sock, SIOCGIFINDEX, &ifr);
    
    struct sockaddr_can addr;
    addr.can_family = AF_CAN;
    addr.can_ifindex = ifr.ifr_ifindex;
    bind(sock, (struct sockaddr*)&addr, sizeof(addr));
    
    struct can_frame frame;
    
    while(true) {
        int nbytes = read(sock, &frame, sizeof(frame));
        
        if(nbytes > 0 && frame.can_id == 0x101) {
            uint8_t percentage = frame.data[0];
            std::cout << "Battery: " << (int)percentage << "%" << std::endl;
        }
    }
    
    close(sock);
    return 0;
}
```

---

## Troubleshooting

### Problem: CAN_Init() returns HAL_ERROR

**Possible Causes**:
1. FDCAN1 not enabled in CubeMX
2. Clock configuration incorrect
3. Pins not configured

**Solution**:
```c
// Check if FDCAN is enabled
if (HAL_FDCAN_Start(&hfdcan1) != HAL_OK) {
    // Check error state
    uint32_t error = HAL_FDCAN_GetError(&hfdcan1);
    // Debug: print error code
}
```

### Problem: Messages not received on Raspberry Pi

**Debugging Steps**:

1. **Check Physical Connections**:
   ```bash
   # On Raspberry Pi, check CAN interface
   ifconfig can0
   # Should show UP and RUNNING
   ```

2. **Monitor CAN Bus**:
   ```bash
   candump can0
   # Should show: can0  101   [1]  55  (for 85%)
   ```

3. **Check Bitrate**:
   ```bash
   ip -details link show can0
   # Verify bitrate matches STM32 (500000)
   ```

4. **Test with cansend**:
   ```bash
   # Send test message from RPi
   cansend can0 101#AA
   # STM32 should receive 0xAA
   ```

### Problem: CAN_SendBattery() returns HAL_ERROR

**Possible Causes**:
1. TX FIFO full
2. Bus off state
3. Hardware error

**Solution**:
```c
// Check FIFO status
uint32_t free_level = HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1);
if (free_level == 0) {
    Debug_Print("TX FIFO full!\r\n");
}

// Check bus state
uint32_t protocol_status = HAL_FDCAN_GetProtocolStatus(&hfdcan1);
// Decode status bits...
```

### Problem: No termination resistors

**Symptoms**:
- Messages not received
- Bus errors
- Inconsistent communication

**Solution**:
Add 120Ω resistors:
```
  CANH ----[120Ω]---- CANL  (at STM32 end)
  
  CANH ----[120Ω]---- CANL  (at RPi end)
```

### Common Error Codes

| Error Code | Meaning | Solution |
|------------|---------|----------|
| `HAL_ERROR` | General error | Check initialization |
| `HAL_BUSY` | Peripheral busy | Wait and retry |
| `HAL_TIMEOUT` | Operation timeout | Check bus connection |

---

## Performance Considerations

### Timing

- **Message transmission time**: ~200 µs at 500 kbit/s
- **Recommended send interval**: ≥100 ms (to avoid bus saturation)
- **Current implementation**: 2 seconds (very conservative)

### Bus Load Calculation

```
Bus Load = (Message Size × Messages/Second) / Bitrate

For our implementation:
- Message size: ~100 bits (including overhead)
- Rate: 0.5 messages/second (every 2 seconds)
- Bus load: (100 × 0.5) / 500000 = 0.01% (very low)
```

### Optimization Tips

1. **Batch multiple readings**: Send voltage, current, and percentage in one 8-byte message
2. **Use appropriate priority**: Lower CAN ID for critical messages
3. **Implement error handling**: Retry on failures
4. **Monitor bus load**: Keep total load under 70%

---

## Additional Resources

- [STM32U5 FDCAN Reference Manual](https://www.st.com/resource/en/reference_manual/rm0456-stm32u5-series-armbased-32bit-mcus-stmicroelectronics.pdf)
- [CAN Bus Specification (ISO 11898)](https://www.iso.org/standard/63648.html)
- [Python-CAN Documentation](https://python-can.readthedocs.io/)
- [Linux SocketCAN Documentation](https://www.kernel.org/doc/html/latest/networking/can.html)

---

## License

This code is provided as-is for educational and development purposes.
