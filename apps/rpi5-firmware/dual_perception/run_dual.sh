LANE_HEF=$HOME/Lka/apps/rpi5-firmware/perception/trained_models/yolov8n_seg_100e_test.hef
DET_HEF=$HOME/tsr/models/yolov8s_oriented_cars_v2.hef
TSR_LABELS=$HOME/tsr/models/labels.txt

python3 run_dual_pipeline.py --seg-hef $LANE_HEF --det-hef $DET_HEF --labels $TSR_LABELS
