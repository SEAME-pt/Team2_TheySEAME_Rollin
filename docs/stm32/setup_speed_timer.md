# Using a Timer as a Tachometer for RPM Measurement on B-U585I-IOT02A

This guide explains how to set up and use **TIM1 Channel 3 (PE13)** on the **B-U585I-IOT02A** development board to measure RPM using a tachometer. The tachometer generates pulses proportional to the rotational speed, and the STM32's timer captures these pulses to calculate RPM.

---

## **Prerequisites**
- **Hardware**:
  - B-U585I-IOT02A development board
  - Tachometer sensor connected to **PE13 (TIM1 Channel 3)**
- **Software**:
  - STM32CubeIDE
  - STM32CubeMX (optional, for graphical configuration)

---

## **Step 1: Configure the Timer in Input Capture Mode**

### **1.1 Enable TIM1 Clock**
Ensure the **TIM1 clock** is enabled in your project. In STM32CubeIDE, this is typically done automatically when you configure TIM1.

### **1.2 Configure GPIO for TIM1 Channel 3**
- **Pin**: **PE13** (TIM1 Channel 3)
- **Mode**: **Alternate Function (AF)**
- **Alternate Function**: **AF1** (for TIM1)

In STM32CubeIDE:
1. Open the `.ioc` file for your project.
2. Select **PE13** and set its mode to **Alternate Function (AF1)**.

### **1.3 Configure TIM1 in Input Capture Mode**
1. Open **TIM1** configuration in STM32CubeIDE.
2. Set the **Clock Source** to **Internal Clock**.
3. Configure **Channel 3** for **Input Capture direct mode**:
   - **Polarity**: Rising edge (or falling edge, depending on your tachometer signal)
   - **Prescaler**: Set based on your expected RPM range
   - **Filter**: Optional, to reduce noise
4. Enable **TIM1 global interrupt - Capture Compare Interrupt in NVIC settings** for input capture events.

---

## **Step 2: Initialize the Timer in Code**

### **2.1 Initialize TIM1 in Input Capture Mode**
```c
#include "stm32u5xx_hal.h"

TIM_HandleTypeDef htim1;

void TIM1_Init(void) {
  TIM_IC_InitTypeDef sConfigIC = {0};

  // Enable TIM1 clock
  __HAL_RCC_TIM1_CLK_ENABLE();

  // Configure TIM1
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 8399; // Example: For 100 µs tick period (adjust as needed)
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 0xFFFF; // Maximum period for 16-bit timer
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  HAL_TIM_IC_Init(&htim1);

  // Configure Channel 3 for Input Capture
  sConfigIC.ICPolarity = TIM_INPUTCHANNELPOLARITY_RISING; // or FALLING
  sConfigIC.ICSelection = TIM_ICSELECTION_DIRECTTI;
  sConfigIC.ICPrescaler = TIM_ICPSC_DIV1; // No input prescaler
  sConfigIC.ICFilter = 0; // No filter
  HAL_TIM_IC_ConfigChannel(&htim1, &sConfigIC, TIM_CHANNEL_3);

  // Start TIM1 in Input Capture Interrupt Mode
  HAL_TIM_IC_Start_IT(&htim1, TIM_CHANNEL_3);
}

          prev_capture = curr_capture;
      }
  }
