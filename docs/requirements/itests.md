# Integration Tests

## Comprehensive Cluster System Test

```
`itest~cluster-system-integration~1`

Status: proposed

Description: Comprehensive verification of cluster system including startup sequence, real-time data display, weather/time information, and UI functionality.

Test Step:
1. Start the QT cluster application and measure startup sequence
2. Verify manufacturer logo (They SEA:ME Rolling Team) is displayed for ≤4 seconds
3. Verify UI readability and font sizing under various lighting conditions
4. Start the STM32 sensor module and RPi5 for real-time data testing
5. Set throttle to stable value (e.g., 100%) and allow motor speed to stabilize
6. Measure tachometer frequency using oscilloscope over 10-second period
7. Calculate expected speed from tachometer frequency using wheel diameter and gear ratio
8. Compare cluster-displayed speed to oscilloscope-calculated speed
9. Measure the current around the INA219 to manually compare battery values
10. Generate test CAN messages with gear changes (P, N, D, R) using PCAN
11. Verify cluster displays speed with ≤100ms latency and 10Hz refresh rate
12. Verify gear display updates correctly within 100ms of CAN message
13. Verify battery display with ≤10s latency at 0.1Hz refresh rate
14. Test weather information fetch from Open-Meteo API with network connectivity
15. Verify time display in HH:MM format and updates every second
16. Test graceful handling of network disconnections

Expected Result: 
- Manufacturer logo displayed for ≤4 seconds on startup
- Critical information displayed with readable font and size under all lighting conditions
- Speed displayed in hm/h matches oscilloscope-measured tachometer frequency (±5% tolerance) with ≤100ms latency at 10Hz refresh rate
- Battery displayed as percentage with ≤10s latency at 0.1Hz refresh rate
- Gear displayed as single letter (P, N, D, R) with ≤100ms latency
- Weather information displayed and updated per API refresh rate
- Time displayed in HH:MM format and updated every second
- Graceful handling of network disconnections

Covers:
- `dsn~manufacturer-logo-display~1`
- `dsn~gui-readable-font-sizing~1`
- `dsn~display-speed-real-time~1`
- `dsn~display-battery-real-time~1`
- `dsn~cluster-gear-display~1`
- `dsn~current-weather-display~1`

Evidence: `docs/requirements/evidence/cluster_system_integration_test/`
- `startup_sequence_video.mp4`
- `logo_timing_measurements.txt`
- `ui_readability_screenshots/`
- `lighting_condition_tests/`
- `oscilloscope_tachometer_frequency.png`
- `speed_calculation_worksheet.xlsx`
- `cluster_speed_display_screenshot.png`
- `battery_display_0.1hz_log.txt`
- `gear_display_screenshots/`
- `can_message_logs/`
- `timing_analysis.xlsx`
- `weather_api_response_logs.json`
- `time_update_verification.txt`
- `network_disconnect_behavior.log`
- `weather_display_screenshots/`

```

## Integrated Vehicle Control and Data System Test

```
`itest~vehicle-control-data-integration~1`

Status: proposed

Description: Comprehensive verification of vehicle control system including gear modes, motor behavior, sensor data collection, and CAN communication integration.

Test Steps:
1. Configure CAN bus at 500 kbps bit rate using CAN 2.0B
2. Start STM32 sensor modules and verify sensor data collection
3. Monitor speed sensor data collection at 10Hz
4. Monitor battery sensor data collection at least once every 2 seconds
5. Verify data accuracy by comparing with known reference values
6. Test CAN message transmission with correct IDs and periods
7. Monitor CAN bus utilization under normal operating conditions
8. Test heartbeat message transmission from RPi5 to STM32
9. Set gear to Park (P) mode and verify motor disabled, throttle at 0%
10. Set gear to Neutral (N) mode and verify motor disabled, throttle at 0%
11. Set gear to Drive (D) mode and test bidirectional motor control
12. Set gear to Reverse (R) mode and verify backward-only motor control
13. Verify gear transitions are smooth and safe
14. Test sensor data transmission over CAN bus during all gear modes
15. Verify timing consistency over extended periods
16. Test heartbeat loss behavior and system response
17. Measure actual vs. specified transmission periods for all messages

Expected Result:
- CAN bus operates at 500 kbps with CAN 2.0B protocol
- All messages transmitted with correct IDs and periods
- Bus utilization ≤40% under normal conditions
- Speed measurements updated at 10Hz frequency
- Battery measurements updated at least once every 2 seconds (≥0.5Hz)
- Sensor data transmitted accurately over CAN bus
- P mode: Motor control disabled, throttle locked at 0%
- N mode: Motor control disabled, throttle locked at 0%
- D mode: Motor control enabled in both directions
- R mode: Motor control enabled in backward direction only, forward commands ignored
- Heartbeat transmitted within specified timing tolerance
- System detects heartbeat loss and activates appropriate responses
- Consistent timing performance over extended operation

Covers:
- `dsn~can-bus-configuration~1`
- `dsn~can-message-definitions~1`
- `dsn~can-bus-load~1`
- `dsn~heartbeat-transmission~1`
- `dsn~speed-measure-device~1`
- `dsn~battery-measure-device~1`
- `dsn~gear-park-behavior~1`
- `dsn~gear-neutral-behavior~1`
- `dsn~gear-drive-behavior~1`
- `dsn~gear-reverse-behavior~1`

Evidence: `docs/requirements/evidence/vehicle_control_data_integration_test/`
- `can_bus_config_verification.txt`
- `message_id_period_analysis.xlsx`
- `bus_utilization_monitoring.log`
- `heartbeat_timing_measurements.txt`
- `can_trace_logs/`
- `bus_load_analysis_charts/`
- `speed_sensor_10hz_timing_log.txt`
- `battery_sensor_timing_log.txt`
- `sensor_accuracy_comparison.xlsx`
- `can_transmission_verification.log`
- `extended_operation_timing_analysis.xlsx`
- `sensor_calibration_results/`
- `park_mode_motor_disabled_log.txt`
- `neutral_mode_motor_disabled_log.txt`
- `drive_mode_bidirectional_log.txt`
- `reverse_mode_backward_only_log.txt`
- `motor_control_can_traces/`
- `gear_transition_videos/`

```

## Integrated Safety and Emergency Response Test

```
`itest~safety-emergency-response-integration~1`

Status: proposed

Description: Comprehensive verification of safety systems and emergency response including remote control emergency functions and safety module authority.

Test Steps:
1. Connect remote control to vehicle system
2. Test throttle control across full range (0-100%)
3. Test steering control across full range (left-right)
4. Test gear selection between P, N, D, R modes via remote control
5. Test control responsiveness and latency for normal operations
6. Set up test environment with controllable obstacle detection
7. Send normal throttle and steering commands via remote control
8. Trigger obstacle detection within safety threshold
9. Verify safety module activates FAILSAFE mode within 10ms
10. Verify vehicle stops immediately (throttle 0%, safe braking activated)
11. Test emergency braking activation via remote control
12. Verify emergency braking immediately stops vehicle regardless of other inputs
13. Verify emergency braking overrides all other commands
14. Verify safety override works regardless of perception module status
15. Test safety module authority during various operating scenarios
16. Test recovery from FAILSAFE mode
17. Verify control latency ≤100ms for safety-critical functions

Expected Result:
- Remote control updates throttle, steering, and gear selection with ≤100ms latency
- Emergency braking immediately stops vehicle regardless of other inputs
- Control latency ≤100ms for safety-critical functions
- Safety module detects obstacles and activates FAILSAFE mode within 10ms
- Vehicle motion immediately stopped with throttle set to 0% and safe braking
- Safety module overrides perception module commands when necessary
- Emergency braking via remote control overrides all other commands
- System can safely recover from FAILSAFE mode when obstacles cleared

Covers:
- `dsn~remote-control-functions~1`
- `dsn~safety-module-authority~1`

Evidence: `docs/requirements/evidence/safety_emergency_response_integration_test/`
- `throttle_range_test_log.txt`
- `steering_range_test_log.txt`
- `gear_selection_test_log.txt`
- `control_latency_measurements.xlsx`
- `remote_control_demo_video.mp4`
- `emergency_brake_response_time.txt`
- `obstacle_detection_response_time.txt`
- `failsafe_activation_log.txt`
- `vehicle_stopping_distance_measurements.xlsx`
- `safety_override_test_videos/`
- `failsafe_recovery_procedures.log`
- `emergency_stop_can_traces/`
- `remote_emergency_brake_tests/`

```