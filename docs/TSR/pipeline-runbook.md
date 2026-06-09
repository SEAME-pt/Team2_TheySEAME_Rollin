# TSR Pipeline Run/Stop Commands

These commands are executed from your local machine to control the Raspberry Pi pipeline over SSH.

## Start (window + named pipe)

```bash
ssh root@10.21.221.17 "rm -f /root/tsr/NamedPipeTsr; mkfifo /root/tsr/NamedPipeTsr; chmod 666 /root/tsr/NamedPipeTsr; cd /root/tsr; XDG_RUNTIME_DIR=/run/user/200 WAYLAND_DISPLAY=wayland-1 nohup python3 run_ai_pipeline.py --hef /root/tsr/models/yolov8s.hef --labels /root/tsr/models/labels.txt --pipe /root/tsr/NamedPipeTsr > /root/tsr/run_tsr.log 2>&1 < /dev/null &"
```

## Start (named pipe only, no window)

```bash
ssh root@10.21.221.17 "rm -f /root/tsr/NamedPipeTsr; mkfifo /root/tsr/NamedPipeTsr; chmod 666 /root/tsr/NamedPipeTsr; cd /root/tsr; nohup python3 run_ai_pipeline.py --no-display --hef /root/tsr/models/yolov8s.hef --labels /root/tsr/models/labels.txt --pipe /root/tsr/NamedPipeTsr > /root/tsr/run_tsr.log 2>&1 < /dev/null &"
```

## Stop

```bash
ssh root@10.21.221.17 "pids=$(ps aux | grep -F run_ai_pipeline.py | grep -v grep | awk '{print $2}'); if [ -n \"$pids\" ]; then kill $pids; fi"
```

## Check Status

```bash
ssh root@10.21.221.17 "ps aux | grep -F run_ai_pipeline.py | grep -v grep"
```

## View Logs

```bash
ssh root@10.21.221.17 "tail -n 50 /root/tsr/run_tsr.log"
```

