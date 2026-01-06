Unit Test Definitions
---------------------

### Requirement:  dsg-control-throttle-command-1
---------------------

### **UT-CSA-01 – Zero Angle Conversion**

**Purpose:** Verify correct PWM output for neutral steering angle.

*   **Input:** 0°
    
*   **Expected Behavior:**
    
    *   No clamping applied
        
    *   PWM = 307 counts
        
    *   Driver Module is commanded with 307
        

### **UT-CSA-02 – Maximum Positive Angle**

**Purpose:** Verify conversion at upper safe limit.

*   **Input:** +30°
    
*   **Expected Behavior:**
    
    *   No clamping applied
        
    *   PWM = 307 + (30 × 2.27) = 375.1 ≈ 375 counts
        
    *   Driver Module is commanded with the calculated PWM
        

### **UT-CSA-03 – Maximum Negative Angle**

**Purpose:** Verify conversion at lower safe limit.

*   **Input:** -30°
    
*   **Expected Behavior:**
    
    *   No clamping applied
        
    *   PWM = 307 − (30 × 2.27) = 238.9 ≈ 239 counts
        
    *   Driver Module is commanded with the calculated PWM
        

### **UT-CSA-04 – Positive Angle Above Limit**

**Purpose:** Verify safety clamping for values above maximum range.

*   **Input:** +45°
    
*   **Expected Behavior:**
    
    *   Input is clamped to +30°
        
    *   PWM calculated using +30°
        
    *   Driver Module is commanded with clamped PWM value
        

### **UT-CSA-05 – Negative Angle Below Limit**

**Purpose:** Verify safety clamping for values below minimum range.

*   **Input:** -50°
    
*   **Expected Behavior:**
    
    *   Input is clamped to -30°
        
    *   PWM calculated using -30°
        
    *   Driver Module is commanded with clamped PWM value
        

### **UT-CSA-06 – Small Positive Angle**

**Purpose:** Verify conversion accuracy for small positive values.

*   **Input:** +1°
    
*   **Expected Behavior:**
    
    *   No clamping applied
        
    *   PWM = 307 + 2.27 ≈ 309 counts
        
    *   Driver Module is commanded with calculated PWM
        

### **UT-CSA-07 – Small Negative Angle**

**Purpose:** Verify conversion accuracy for small negative values.

*   **Input:** -1°
    
*   **Expected Behavior:**
    
    *   No clamping applied
        
    *   PWM = 307 − 2.27 ≈ 305 counts
        
    *   Driver Module is commanded with calculated PWM
        

### **UT-CSA-08 – Driver Command Invocation**

**Purpose:** Verify that the Driver Module is always commanded.

*   **Input:** Any valid angle
    
*   **Expected Behavior:**
    
    *   Driver Module command function is invoked exactly once
        
    *   PWM value matches computed result
        

### **UT-CSA-09 – Interface Range Acceptance**

**Purpose:** Verify interface accepts full specified input range.

*   **Input:** Any value in \[-30°, +30°\]
    
*   **Expected Behavior:**
    
    *   No error or rejection
        
    *   Correct PWM conversion
        
    *   Driver Module is commanded

### Requirement: dsg-control-throttle-command-1
---------------------

### **UT-CTC-01 – Zero Speed Command**

**Purpose:** Verify neutral throttle behavior.

*   **Input:** Speed command = 0, Direction = Forward
    
*   **Expected Behavior:**
    
    *   PWM output corresponds to zero motor speed
        
    *   Driver Module is commanded with neutral PWM
        
    *   No vehicle motion is requested
        

### **UT-CTC-02 – Forward Motion Command**

**Purpose:** Verify forward direction control.

*   **Input:** Positive speed command within allowed range, Direction = Forward
    
*   **Expected Behavior:**
    
    *   PWM output proportional to requested speed
        
    *   Driver Module is commanded with forward direction
        
    *   Vehicle motion command indicates forward movement
        

### **UT-CTC-03 – Reverse Motion Command**

**Purpose:** Verify reverse direction control.

*   **Input:** Positive speed command within allowed range, Direction = Reverse
    
*   **Expected Behavior:**
    
    *   PWM output proportional to requested speed
        
    *   Driver Module is commanded with reverse direction
        
    *   Vehicle motion command indicates reverse movement
        

### **UT-CTC-04 – Maximum Forward Speed Limit**

**Purpose:** Verify enforcement of upper speed limit in forward direction.

*   **Input:** Speed command above maximum allowed, Direction = Forward
    
*   **Expected Behavior:**
    
    *   Speed command is clamped to maximum allowed value
        
    *   PWM corresponds to maximum forward speed
        
    *   Driver Module is commanded with clamped PWM
        

### **UT-CTC-05 – Maximum Reverse Speed Limit**

**Purpose:** Verify enforcement of upper speed limit in reverse direction.

*   **Input:** Speed command above maximum allowed, Direction = Reverse
    
*   **Expected Behavior:**
    
    *   Speed command is clamped to maximum allowed value
        
    *   PWM corresponds to maximum reverse speed
        
    *   Driver Module is commanded with clamped PWM
        

### **UT-CTC-06 – Invalid Direction Handling**

**Purpose:** Verify safe behavior for invalid direction input.

*   **Input:** Valid speed command, invalid direction value
    
*   **Expected Behavior:**
    
    *   Safety constraint is applied
        
    *   Motor output is disabled or set to zero speed
        
    *   Driver Module is commanded with safe PWM
        

### **UT-CTC-07 – Speed Below Minimum Threshold**

**Purpose:** Verify handling of very small speed commands.

*   **Input:** Speed command below minimum effective threshold
    
*   **Expected Behavior:**
    
    *   Speed is treated as zero
        
    *   PWM output corresponds to zero motor speed
        
    *   No unintended motion is commanded
        

### **UT-CTC-08 – Driver Command Invocation**

**Purpose:** Verify Driver Module interaction.

*   **Input:** Any valid speed and direction command
    
*   **Expected Behavior:**
    
    *   Driver Module PWM command function is invoked exactly once
        
    *   PWM value and direction match computed results
        

### **UT-CTC-09 – Safety Constraint Enforcement**

**Purpose:** Verify throttle safety constraints are always enforced.

*   **Input:** Any command violating defined safety limits
    
*   **Expected Behavior:**
    
    *   Unsafe command is clamped or rejected
        
    *   Motor output remains within safe operating range
        
    *   Driver Module is commanded with safe values only

### Requirement: dsg-control-safety-limits-1
---------------------

### **UT-CSL-01 – Steering Upper Limit Enforcement**

**Purpose:** Verify enforcement of the maximum steering angle limit.

*   **Input:** Steering angle command greater than +30°
    
*   **Expected Behavior:**
    
    *   Steering command is clamped to +30°
        
    *   Actuator command corresponds to the clamped value
        
    *   No unsafe steering command is issued
        

### **UT-CSL-02 – Steering Lower Limit Enforcement**

**Purpose:** Verify enforcement of the minimum steering angle limit.

*   **Input:** Steering angle command less than -30°
    
*   **Expected Behavior:**
    
    *   Steering command is clamped to -30°
        
    *   Actuator command corresponds to the clamped value
        
    *   No unsafe steering command is issued
        

### **UT-CSL-03 – Speed Upper Limit Enforcement**

**Purpose:** Verify enforcement of maximum allowed speed.

*   **Input:** Speed command above the defined safe operational range
    
*   **Expected Behavior:**
    
    *   Speed command is clamped to the maximum allowed value
        
    *   PWM output remains within safe limits
        
    *   Motor actuator is commanded safely
        

### **UT-CSL-04 – Speed Lower Limit Enforcement**

**Purpose:** Verify enforcement of minimum or zero speed limit.

*   **Input:** Speed command below minimum allowed (e.g. negative when not allowed)
    
*   **Expected Behavior:**
    
    *   Speed command is forced to zero or minimum safe value
        
    *   Motor PWM corresponds to safe output
        
    *   No unintended vehicle motion occurs
        

### **UT-CSL-05 – Emergency Stop Activation**

**Purpose:** Verify emergency stop overrides all control commands.

*   **Input:** Emergency stop signal asserted
    
*   **Expected Behavior:**
    
    *   Steering output is disabled or set to safe neutral position
        
    *   Throttle output is set to zero speed
        
    *   Driver Module receives only safe actuator commands
        

### **UT-CSL-06 – Emergency Stop Priority**

**Purpose:** Verify emergency stop has highest priority.

*   **Input:** Valid steering and speed commands with emergency stop asserted
    
*   **Expected Behavior:**
    
    *   Normal control commands are ignored
        
    *   Emergency stop behavior is applied immediately
        
    *   No actuator follows user commands
        

### **UT-CSL-07 – Input Validation Before Actuation**

**Purpose:** Verify validation of control inputs before actuator commands.

*   **Input:** Any control command set (steering or speed)
    
*   **Expected Behavior:**
    
    *   Inputs are validated against safety limits
        
    *   Only validated and safe commands are forwarded
        
    *   Invalid or out-of-range inputs are not sent to actuators
        

### **UT-CSL-08 – Combined Steering and Speed Limits**

**Purpose:** Verify simultaneous enforcement of multiple safety limits.

*   **Input:** Steering angle and speed commands both exceeding limits
    
*   **Expected Behavior:**
    
    *   Each command is independently clamped to safe range
        
    *   Actuator commands reflect safe values only
        
    *   System remains in a valid operational state
        

### **UT-CSL-09 – Fail-Safe Behavior on Invalid Input**

**Purpose:** Verify fail-safe behavior when input validation fails.

*   **Input:** Corrupted or undefined control command values
    
*   **Expected Behavior:**
    
    *   System transitions to a safe state
        
    *   Throttle is set to zero speed
        
    *   Steering is set to neutral or last known safe value
        
    *   No unsafe actuator command is ever issued
        
    *   System behavior is deterministic and repeatable

### Requirement: dsg-control-user-interface-1
---------------------
### **UT-CUI-01 – Steering Command Reception**

**Purpose:** Verify reception of steering angle commands from the Communication Module.

*   **Input:** Valid steering angle command message
    
*   **Expected Behavior:**
    
    *   Steering command is correctly received
        
    *   Value is forwarded to the steering control logic
        
    *   No data loss or modification occurs
        

### **UT-CUI-02 – Throttle Command Reception**

**Purpose:** Verify reception of throttle/speed commands from the Communication Module.

*   **Input:** Valid throttle or speed command message
    
*   **Expected Behavior:**
    
    *   Throttle command is correctly received
        
    *   Value is forwarded to the throttle control logic
        
    *   No data corruption occurs
        

### **UT-CUI-03 – Mode Selection Reception**

**Purpose:** Verify reception of mode selection commands.

*   **Input:** Mode command = Manual
    
*   **Expected Behavior:**
    
    *   Mode is correctly parsed and stored
        
    *   Control Module switches to manual mode
        

### **UT-CUI-04 – Autonomous Mode Selection**

**Purpose:** Verify autonomous mode selection handling.

*   **Input:** Mode command = Autonomous
    
*   **Expected Behavior:**
    
    *   Mode is correctly parsed and stored
        
    *   Control Module switches to autonomous mode
        

### **UT-CUI-05 – Command Routing in Manual Mode**

**Purpose:** Verify correct routing of user commands in manual mode.

*   **Input:** Manual mode active, valid steering and throttle commands
    
*   **Expected Behavior:**
    
    *   Commands are routed to manual control logic
        
    *   Autonomous control logic is not invoked
        

### **UT-CUI-06 – Command Routing in Autonomous Mode**

**Purpose:** Verify correct routing of commands in autonomous mode.

*   **Input:** Autonomous mode active, steering and throttle commands
    
*   **Expected Behavior:**
    
    *   Commands are routed to autonomous control logic
        
    *   Manual control logic is not invoked
        

### **UT-CUI-07 – Input Message Validation**

**Purpose:** Verify validation of received communication messages.

*   **Input:** Malformed or incomplete command message
    
*   **Expected Behavior:**
    
    *   Message is rejected
        
    *   No control action is triggered
        
    *   System remains in a safe state
        

### **UT-CUI-08 – Unsupported Command Handling**

**Purpose:** Verify handling of unsupported or unknown command types.

*   **Input:** Unknown command ID from Communication Module
    
*   **Expected Behavior:**
    
    *   Command is ignored or rejected
        
    *   No unintended control behavior occurs
        

### **UT-CUI-09 – Command Update Consistency**

**Purpose:** Verify consistent processing of successive commands.

*   **Input:** Sequence of valid steering, throttle, and mode commands
    
*   **Expected Behavior:**
    
    *   Each command is processed in order
        
    *   Latest valid command state is maintained
        

### **UT-CUI-10 – Communication Module Interface Invocation**

**Purpose:** Verify integration with the Communication Module interface.

*   **Input:** Any valid command message
    
*   **Expected Behavior:**
    
    *   Communication interface callback or handler is invoked exactly once
        
    *   Control Module receives the correct command payload
