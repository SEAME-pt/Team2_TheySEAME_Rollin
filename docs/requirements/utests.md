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
        

### **UT-CTC-10 – Interface Acceptance**

**Purpose:** Verify throttle control interface acceptance.

*   **Input:** Any speed/direction command within specified limits
    
*   **Expected Behavior:**
    
    *   No error or rejection
        
    *   Correct PWM and direction computation
        
    *   Driver Module is commanded correctly
