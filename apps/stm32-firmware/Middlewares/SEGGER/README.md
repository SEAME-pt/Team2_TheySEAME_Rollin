# SEGGER SystemView Profiling — STM32U585 + ThreadX

Post-mortem thread profiling via OpenOCD RAM dump (ST-Link, no J-Link needed).

## Usage

```bash
cd apps/stm32-firmware

# 1. Capture (~2s snapshot of thread activity)
./dump_sysview.sh

# 2. Open the generated .SVDat file in SystemView
/opt/SEGGER/SystemView_V362c/SystemView $(ls -t sysview_*.SVDat | head -1)
```

- Click **OK** on the "Error -161" warning — normal for post-mortem captures.
- **Do NOT use the Record button** — it requires J-Link. Only use **File → Open**.

## What You See

- **Events List**: every context switch and ISR, microsecond timestamps
- **Timeline**: which thread was running on the CPU at each moment
- **CPU Load**: per-thread usage percentage
- **Context Statistics** tab: min/max/avg execution times per thread

Threads: Battery (pri 10), Communication (pri 10), Control (pri 8), Sensors Proc (pri 12), Speed (pri 14), System Timer.

## Rebuilding After CubeIDE Regenerates Makefiles

CubeIDE overwrites makefiles and drops custom flags. After regeneration:

```bash
./fix_segger_build.sh
```

Also verify `Debug/Middlewares/ST/threadx/ports/cortex_m33/gnu/src/subdir.mk` has `-DTX_INCLUDE_USER_DEFINE_FILE` in the assembly compile rule — CubeIDE removes it.

## Key Files

| File | Purpose |
|------|---------|
| `dump_sysview.sh` | Halt CPU + dump RTT buffer via OpenOCD |
| `Config/SEGGER_SYSVIEW_Conf.h` | Buffer size (16 KB), post-mortem mode |
| `Config/SEGGER_SYSVIEW_Config_ThreadX.c` | SystemView init, DWT cycle counter |
| `Config/SEGGER_SYSVIEW_ThreadX.c` | ThreadX scheduler hooks → SystemView |
| `Config/SEGGER_RTT_Conf.h` | RTT buffer config, lock macros |

## Memory

- Flash: ~15 KB
- RAM: ~16 KB (event buffer)
- CPU overhead: ~2-3%

