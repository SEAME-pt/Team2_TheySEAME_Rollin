# Integration Tests

## Cluster Display and Real-Time Data Tests

```
`itest~cluster-realtime-data-display~1`

**Status**: proposed

**Description**: Verify that the cluster displays real-time vehicle data (speed, battery, gear) with correct refresh rates and accuracy.

**Test Steps**:
1. Start the STM32 sensor module, RPi5 and QT cluster application
2. Set throttle to stable value (e.g., 100%) and allow motor speed to stabilize
3. Measure tachometer frequency using oscilloscope over 10-second period
4. Calculate expected speed from tachometer frequency using wheel diameter and gear ratio
5. Compare cluster-displayed speed to oscilloscope-calculated speed
6. Measure the current around the INA219 to manually compare battery values using the same calculation as in the code
7. PCAN Generate test CAN messages with gear changes (P, N, D, R)
8. Verify cluster displays speed with ≤100ms latency and 10Hz refresh rate
9. Verify gear display updates correctly within 100ms of CAN message

**Expected Result**: 

- Speed displayed in hm/h matches oscilloscope-measured tachometer frequency (±5% tolerance) with ≤100ms latency at 10Hz refresh rate
- Battery displayed as percentage with ≤10s latency at 0.1Hz refresh rate
- Gear displayed as single letter (P, N, D, R) with ≤100ms latency

**Covers**:
- `dsn~display-speed-real-time~1`
- `dsn~display-battery-real-time~1`
- `dsn~cluster-gear-display~1`

**Evidence**: `docs/requirements/evidence/cluster_realtime_data_test/`
- `oscilloscope_tachometer_frequency.png`
- `speed_calculation_worksheet.xlsx`
- `cluster_speed_display_screenshot.png`
- `battery_display_0.1hz_log.txt`
- `gear_display_screenshots/`
- `can_message_logs/`
- `timing_analysis.xlsx`
```

```
`itest~cluster-startup-and-ui~1`

**Status**: proposed

**Description**: Verify cluster startup sequence, manufacturer logo display, and UI readability requirements.

**Test Steps**:
1. Start the QT cluster application
2. Measure logo display duration from startup
3. Verify manufacturer logo (They SEA:ME Rolling Team) is displayed
4. Verify logo disappears after maximum 4 seconds
5. Verify font sizing and readability of critical information

**Expected Result**:
- Manufacturer logo displayed for ≤4 seconds on startup
- Critical information displayed with readable font and size
- UI elements clearly visible under various lighting conditions

**Covers**:
- `dsn~manufacturer-logo-display~1`
- `dsn~gui-readable-font-sizing~1`

**Evidence**: `docs/requirements/evidence/cluster_startup_ui_test/`
- `startup_sequence_video.mp4`
- `logo_timing_measurements.txt`
- `ui_readability_screenshots/`
- `lighting_condition_tests/`
```

```
`itest~cluster-weather-time-display~1`

**Status**: proposed

**Description**: Verify cluster displays current weather information from Open-Meteo API and system time in HH:MM format.

**Test Steps**:
1. Start cluster application with network connectivity
2. Verify weather information is fetched from Open-Meteo API
3. Verify weather updates according to API refresh rate
4. Verify time is displayed in HH:MM format
5. Verify time updates every minute
6. Test behavior with network disconnection

**Expected Result**:
- Weather information displayed and updated per API refresh rate
- Time displayed in HH:MM format and updated every second
- Graceful handling of network disconnections

**Covers**:
- `dsn~current-weather-display~1`

**Evidence**: `docs/requirements/evidence/cluster_weather_time_test/`
- `weather_api_response_logs.json`
- `time_update_verification.txt`
- `network_disconnect_behavior.log`
- `weather_display_screenshots/`

```

## Gear Control and Motor Behavior Tests

```
`itest~gear-mode-motor-control~1`

**Status**: proposed

**Description**: Verify that different gear modes (P, N, D, R) control motor behavior correctly and safely.

**Test Steps**:
1. Set gear to Park (P) mode
2. Send throttle commands and verify motor is disabled and throttle set to 0%
3. Set gear to Neutral (N) mode
4. Send throttle commands and verify motor is disabled and throttle set to 0%
5. Set gear to Drive (D) mode
6. Send forward and backward throttle commands and verify both directions work
7. Set gear to Reverse (R) mode
8. Send forward throttle commands and verify they are ignored
9. Send backward throttle commands and verify they work
10. Verify gear transitions are smooth and safe

**Expected Result**:
- P mode: Motor control disabled, throttle locked at 0%
- N mode: Motor control disabled, throttle locked at 0%
- D mode: Motor control enabled in both directions
- R mode: Motor control enabled in backward direction only, forward commands ignored

**Covers**:
- `dsn~gear-park-behavior~1`
- `dsn~gear-neutral-behavior~1`
- `dsn~gear-drive-behavior~1`
- `dsn~gear-reverse-behavior~1`

**Evidence**: `docs/requirements/evidence/gear_motor_control_test/`
- `park_mode_motor_disabled_log.txt`
- `neutral_mode_motor_disabled_log.txt`
- `drive_mode_bidirectional_log.txt`
- `reverse_mode_backward_only_log.txt`
- `motor_control_can_traces/`
- `gear_transition_videos/`

**Needs**: impl
```

## Remote Control System Tests

```
`itest~remote-control-functions~1`

**Status**: proposed

**Description**: Verify remote control implements all required control functions for throttle, steering, emergency braking, and gear selection.

**Test Steps**:
1. Connect remote control to vehicle system
2. Test throttle control across full range (0-100%)
3. Test steering control across full range (left-right)
4. Test emergency braking activation
5. Test gear selection between P, N, D, R modes
6. Verify emergency braking overrides other commands
7. Test control responsiveness and latency

**Expected Result**:
- Remote control updates throttle, steering, and gear selection
- Emergency braking immediately stops vehicle regardless of other inputs
- Control latency ≤100ms for safety-critical functions

**Covers**:
- `dsn~remote-control-functions~1`

**Evidence**: `docs/requirements/evidence/remote_control_test/`
- `throttle_range_test_log.txt`
- `steering_range_test_log.txt`
- `emergency_brake_response_time.txt`
- `gear_selection_test_log.txt`
- `control_latency_measurements.xlsx`
- `remote_control_demo_video.mp4`

**Needs**: impl
```

## CAN Bus Communication Tests

```
`itest~can-bus-communication~1`

**Status**: proposed

**Description**: Verify CAN bus configuration, message definitions, bus load, and heartbeat functionality according to specifications.

**Test Steps**:
1. Configure CAN bus at 500 kbps bit rate using CAN 2.0B
2. Verify CAN message IDs and transmission periods according to specification
3. Monitor CAN bus utilization under normal operating conditions
4. Test heartbeat message transmission from Rasp Communications to STM32
5. Verify heartbeat timing tolerance
6. Test system behavior when heartbeat is lost
7. Measure actual vs. specified transmission periods for all messages

**Expected Result**:
- CAN bus operates at 500 kbps with CAN 2.0B protocol
- All messages transmitted with correct IDs and periods
- Bus utilization ≤40% under normal conditions
- Heartbeat transmitted within specified timing tolerance
- System detects heartbeat loss and activates appropriate responses

**Covers**:
- `dsn~can-bus-configuration~1`
- `dsn~can-message-definitions~1`
- `dsn~can-bus-load~1`
- `dsn~heartbeat-transmission~1`

**Evidence**: `docs/requirements/evidence/can_bus_communication_test/`
- `can_bus_config_verification.txt`
- `message_id_period_analysis.xlsx`
- `bus_utilization_monitoring.log`
- `heartbeat_timing_measurements.txt`
- `can_trace_logs/`
- `bus_load_analysis_charts/`

**Needs**: impl
```

## Sensor Data Collection Tests

```
`itest~sensor-data-collection~1`

**Status**: proposed

**Description**: Verify STM32 sensor modules collect and transmit speed and battery data at specified frequencies.

**Test Steps**:
1. Start STM32 sensor modules
2. Monitor speed sensor data collection at 10Hz
3. Monitor battery sensor data collection at least once every 2 seconds
4. Verify data accuracy by comparing with known reference values
5. Test sensor data transmission over CAN bus
6. Verify timing consistency over extended periods
7. Test sensor behavior under various operating conditions

**Expected Result**:
- Speed measurements updated at 10Hz frequency
- Battery measurements updated at least once every 2 seconds (≥0.5Hz)
- Sensor data transmitted accurately over CAN bus
- Consistent timing performance over extended operation

**Covers**:
- `dsn~speed-measure-device~1`
- `dsn~battery-measure-device~1`

**Evidence**: `docs/requirements/evidence/sensor_data_collection_test/`
- `speed_sensor_10hz_timing_log.txt`
- `battery_sensor_timing_log.txt`
- `sensor_accuracy_comparison.xlsx`
- `can_transmission_verification.log`
- `extended_operation_timing_analysis.xlsx`
- `sensor_calibration_results/`

**Needs**: impl
```

## Safety System Tests

```
`itest~safety-module-authority~1`

**Status**: proposed

**Description**: Verify safety module (STM32) has final authority over vehicle motion and can override commands when obstacles are detected.

**Test Steps**:
1. Set up test environment with controllable obstacle detection
2. Send normal throttle and steering commands
3. Trigger obstacle detection within safety threshold
4. Verify safety module activates FAILSAFE mode within 10ms
5. Verify vehicle stops immediately (throttle 0%, safe braking activated)
6. Verify safety override works regardless of perception module status
7. Test safety module authority during various operating scenarios
8. Test recovery from FAILSAFE mode

**Expected Result**:
- Safety module detects obstacles and activates FAILSAFE mode within 10ms
- Vehicle motion immediately stopped with throttle set to 0% and safe braking
- Safety module overrides perception module commands when necessary
- System can safely recover from FAILSAFE mode when obstacles cleared

**Covers**:
- `dsn~safety-module-authority~1`

**Evidence**: `docs/requirements/evidence/safety_module_authority_test/`
- `obstacle_detection_response_time.txt`
- `failsafe_activation_log.txt`
- `vehicle_stopping_distance_measurements.xlsx`
- `safety_override_test_videos/`
- `failsafe_recovery_procedures.log`
- `emergency_stop_can_traces/`

**Needs**: impl
```

---
## Draft Integration Tests for Future Implementation

```
`itest~perception-camera-system~1`

**Status**: draft

**Description**: Verify perception module camera input processing, failure detection, and traffic sign recognition (when implemented).

**Test Steps**:
1. Test camera image capture at specified refresh rate
2. Verify camera failure detection within 100ms
3. Test traffic sign recognition and classification
4. Verify communication between perception and cluster modules
5. Test pedestrian detection capabilities
6. Verify FAILSAFE mode activation on camera failure

**Expected Result**:
- Camera operates at specified refresh rate with quality image capture
- Camera failures detected within 100ms and FAILSAFE activated
- Traffic signs and pedestrians accurately detected and communicated

**Covers**:
- `dsn~perception-camera-input~1`
- `dsn~perception-camera-failure-detection~1`
- `dsn~display-traffic-signs~1`

**Evidence**: `docs/requirements/evidence/perception_camera_test/`
- `camera_refresh_rate_verification.txt`
- `failure_detection_timing.txt`
- `traffic_sign_recognition_results/`
- `pedestrian_detection_logs/`

**Needs**: impl
```

```
`itest~autonomous-navigation-planning~1`

**Status**: draft

**Description**: Verify planning module outputs appropriate steering and throttle commands for autonomous navigation (when implemented).

**Test Steps**:
1. Test planning module response to perception input
2. Verify steering and throttle output commands
3. Test obstacle avoidance behavior
4. Verify emergency stop functionality when obstacles detected

**Expected Result**:
- Planning module generates appropriate steering and throttle commands
- Obstacle avoidance behavior functions correctly
- Emergency stops activated when required

**Covers**:
- `dsn~planning-module-output~1`
- `dsn~planning-obstacle-detection~1`

**Evidence**: `docs/requirements/evidence/autonomous_navigation_test/`
- `planning_output_commands.log`
- `obstacle_avoidance_scenarios/`
- `emergency_stop_response_times.txt`

**Needs**: impl
```

