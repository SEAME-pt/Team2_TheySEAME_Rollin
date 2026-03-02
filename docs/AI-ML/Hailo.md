# HAILO AI HAT
Integrating the HAILO Hat to run AI/ML algorithms for the Perception and Path Planning module.

AI HATs are add-on boards for Raspberry Pi 5 that come with a built-in AI accelerator chip: the Hailo (**neural processing unit (NPU)**)[https://www.ibm.com/think/topics/neural-processing-unit]. The Hailo NPU allows Raspberry Pi 5 to run hardware-accelerated AI models locally, removing the need to send data to 
a remote cloud server for processing. This edge AI approach improves performance, reduces latency, and helps to keep data private. For information about how AI HATs integrate with Raspberry Pi 5 to provide hardware-accelerated AI inference,
see How it works.

## Models
The AI HAT + with 26 TOPS (Tera Operations Per Second) is available to the team and shall be the focus of this document. This is typically used for Object detection, camera post-processing, robotics, moderate neural workloads.

## Hardware Assembly - Active Cooler
Installing and (Active Cooler)[https://mauser.pt/095-2493/raspberry-pi-active-cooler-sc1148-dissipador-com-ventoinha-para-raspberry-pi-5] on the Rasp is reccomended.

## Running models
(This)[products/hailo-software/model-explorer/generative-ai/] page contains ready-made models we can use with the HAILO.
