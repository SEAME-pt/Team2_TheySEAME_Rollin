# SEGGER SystemView Integration for STM32U585 + ThreadX

## What's Been Done ✅

1. **SEGGER Library Files Installed:**
   - `Middlewares/SEGGER/RTT/` - Real-Time Transfer library
   - `Middlewares/SEGGER/SystemView/` - SystemView core library
   - `Middlewares/SEGGER/Config/` - Configuration files for STM32U585

2. **Code Modified:**
   - `Core/Src/app_threadx.c` - Added `SEGGER_SYSVIEW_Conf()` initialization
   - Configuration files created for your specific hardware

## Next Steps - Add Files to STM32CubeIDE Project

### Option 1: Using STM32CubeIDE GUI (Recommended)

1. **Open your project in STM32CubeIDE**

2. **Add SEGGER folders to project:**
   - Right-click on project → **Refresh** (F5)
   - You should now see `Middlewares/SEGGER` folder

3. **Add Include Paths:**
   - Right-click project → **Properties**
   - Navigate to: `C/C++ Build → Settings → MCU GCC Compiler → Include paths`
   - Click **Add** (green + icon) and add:
     ```
     "${workspace_loc:/${ProjName}/Middlewares/SEGGER/RTT}"
     "${workspace_loc:/${ProjName}/Middlewares/SEGGER/SystemView}"
     "${workspace_loc:/${ProjName}/Middlewares/SEGGER/Config}"
     ```
   - Click **Apply**

4. **Add Source Files to Build:**
   - In Project Explorer, expand `Middlewares/SEGGER/`
   - Right-click each `.c` file and select **Add to Build**:
     - `RTT/SEGGER_RTT.c`
     - `SystemView/SEGGER_SYSVIEW.c`
     - `Config/SEGGER_SYSVIEW_Config_ThreadX.c`

5. **Build the project:**
   - Project → **Build Project** (Ctrl+B)
   - Should compile without errors

### Option 2: Manual Build System Modification

If GUI doesn't work, you can manually edit the Debug configuration:

1. **Edit** `Debug/sources.mk`:
   ```makefile
   # Add after existing SUBDIRS:
   SUBDIRS += \
   ../Middlewares/SEGGER/RTT \
   ../Middlewares/SEGGER/SystemView \
   ../Middlewares/SEGGER/Config
   ```

2. **Create** `Debug/Middlewares/SEGGER/RTT/subdir.mk`:
   ```makefile
   C_SRCS += \
   ../Middlewares/SEGGER/RTT/SEGGER_RTT.c
   
   OBJS += \
   ./Middlewares/SEGGER/RTT/SEGGER_RTT.o
   ```

3. **Create** `Debug/Middlewares/SEGGER/SystemView/subdir.mk`:
   ```makefile
   C_SRCS += \
   ../Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.c
   
   OBJS += \
   ./Middlewares/SEGGER/SystemView/SEGGER_SYSVIEW.o
   ```

4. **Create** `Debug/Middlewares/SEGGER/Config/subdir.mk`:
   ```makefile
   C_SRCS += \
   ../Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.c
     
   OBJS += \
   ./Middlewares/SEGGER/Config/SEGGER_SYSVIEW_Config_ThreadX.o
   ```

## Testing SystemView

### 1. Build and Flash Firmware

```bash
# In STM32CubeIDE:
Project → Build Project
Run → Debug (F11)
```

### 2. Open SystemView Application

```bash
# Launch SystemView (already installed)
SystemView
```

### 3. Connect to Target

1. In SystemView: **Target → Start Recording**
2. Select:
   - **Debugger**: ST-Link
   - **Target Device**: STM32U585
   - **Target Interface**: SWD
3. Click **OK**

### 4. Verify Connection

You should see:
- Timeline showing threads: Battery, Control, Communication, Sensors
- CPU load graph
- Event list populating

### 5. Test Emergency Stop Profiling

Once working, you can add custom events in your code:

```c
// In sensor interrupt:
SEGGER_SYSVIEW_RecordU32(100, HAL_GetTick());  // Event ID 100 = sensor trigger

// In Control thread emergency handler:
SEGGER_SYSVIEW_RecordU32(101, HAL_GetTick());  // Event ID 101 = control response
```

Then measure the time between events in SystemView timeline!

## Troubleshooting

### Build Errors

**Error: "SEGGER_SYSVIEW.h: No such file"**
- Solution: Check include paths are added correctly in project properties

**Error: "undefined reference to SEGGER_SYSVIEW_Conf"**
- Solution: Make sure all 3 `.c` files are added to build

### Runtime Issues

**SystemView can't connect:**
- Make sure ST-Link debugger is connected
- Try: Target → Detect Target Device

**No thread information showing:**
- ThreadX integration is basic - you'll see CPU timeline but limited OS info
- This is normal for manual ThreadX integration

## Memory Usage

- Flash: +~15 KB (SystemView library)
- RAM: +~16 KB (event buffer)
- CPU overhead: ~2-3% when recording

## What Next?

Once SystemView is working, you can:
1. Profile your emergency stop response time
2. Identify I2C mutex contention
3. Optimize thread priorities
4. Measure actual CPU usage per thread

Good luck! 🚗💨
