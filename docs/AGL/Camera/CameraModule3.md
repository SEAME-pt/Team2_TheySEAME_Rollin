# Camera Module 3

## Problems

### H264 Codec Problem

The **rpicam-vid** command can't find the codec h264.
As this is the default codec, we need to specify the codec we want to use with ```--codec <video-codec>```.
The only codec that works is the **mjpeg**.

### Camera is upside down

Our camera is installed upside down in the car so, we need to rotate the image/video with ```--rotation 180```

This is an example of using the **rpicam-vid** command

```bash
rpicam-vid --codec mjpeg --rotation 180
```

## Display the camera in the Host PC

To display the camera feed in a Host PC, we need to have **ffmpeg** installed.
Then we can run this command

```bash
ssh root@<Ip-address> "rpicam-vid --codec mjpeg --rotation 180 -t 0 -o -" | ffplay -
```

