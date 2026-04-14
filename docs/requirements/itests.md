# Integration Tests

## Test Case 1 - Comprehensive Cluster System Test

```
`itest~cluster-system-integration~1`

Status: approved

Description: Comprehensive verification of cluster system including startup sequence, real-time data display, weather/time information, and UI functionality.

Preconditions:
- QT cluster application is installed and configured
- Car on the test bench with wheels off the ground
- STM32 sensor module firmware is loaded and functional
- RPi5 system is operational with CAN interface
- Test equipment available: oscilloscope, PCAN interface, INA219 current sensor
- Network connectivity available for weather API testing

Test Steps:
1. - [ ] Start the QT cluster application and measure startup sequence
2. - [ ] Verify manufacturer logo (They SEA:ME Rolling Team) is displayed for ≤4 seconds
    Expected:   Manufacturer logo displayed for ≤4 seconds on startup
3. - [ ] Verify UI readability and font sizing under various lighting conditions
    Expected:   Critical information displayed with readable font and size under all lighting conditions
4. - [ ] Start the STM32 sensor module and RPi5 for real-time data testing
5. - [ ] Set throttle to stable value (e.g., 100%) and allow motor speed to stabilize
6. - [ ] Measure tachometer period between rising or falling edges using oscilloscope five times and average the result
7. - [ ] Calculate expected speed from tachometer period using wheel diameter
8. - [ ] Compare cluster-displayed speed to oscilloscope-calculated speed
    Expected: the displayed speed shall be within a 10% margin of the calculated speed
9. - [ ] Measure the current around the INA219 to manually compare battery values
10. - [ ] Generate test CAN messages with gear changes (P, N, D, R) using PCAN
11. - [ ] Verify cluster displays speed
    Expected:   Speed displayed in hm/h matches oscilloscope-measured tachometer frequency (±5% tolerance)
12. - [ ] Verify gear display updates correctly upon CAN message
    Expected:   Gear displayed as single letter (P, N, D, R) with ≤100ms latency
13. - [ ] Verify battery display
    Expected:   Battery displayed as percentage on QT
14. - [ ] Test weather information fetch from Open-Meteo API with network connectivity
15. - [ ] Verify time display in HH:MM format and updates every second
    Expected:   Weather information displayed and updated per API refresh rate
                Time displayed in HH:MM format and updated every minute
16. - [ ] Test graceful handling of network disconnections
    Expected:   Network disconnection doesn't produce any off-nominal results (loss of weather information may be acceptable, but Time may still show)

Postconditions:
- All cluster display functions verified as operational
- Real-time data accuracy confirmed within specified tolerances
- UI readability validated under various conditions
- System ready for vehicle integration testing

Covers:
- `dsn~manufacturer-logo-display~1`
- `dsn~gui-readable-font-sizing~1`
- `dsn~display-speed-real-time~1`
- `dsn~display-battery-real-time~1`
- `dsn~cluster-gear-display~1`
- `dsn~current-weather-display~1`
- `dsn~speed-measure-device~1`
- `dsn~battery-measure-device~1`



Evidence: `docs/requirements/evidence/cluster_system_integration_test/`
- `startup_sequence_video.mp4`
- `ui_readability_screenshots/`
- `oscilloscope_tachometer_frequency.png`
- `speed_calculation_worksheet.xlsx`
- `battery_display_0.1hz_log.txt`
- `gear_display_screenshots/`
- `can_message_logs/`
- `timing_analysis.xlsx`
- `time_update_verification.txt`
- `network_disconnect_behavior.log`
- `weather_display_screenshots/`

```

## Test Case 2 - Integrated Vehicle Control and Data System Test

```
`itest~vehicle-control-data-integration~1`

Status: approved

Description: Comprehensive verification of vehicle control system including gear modes, motor behavior, sensor data collection, and CAN communication integration.

Preconditions:
- STM32 sensor modules programmed with latest firmware
- RPi5 system configured with CAN interface and mode manager
- CAN bus hardware properly connected and terminated
- Motor control system operational and safely configured
- Test equipment: CAN analyzer, multimeter, oscilloscope
- Vehicle in safe test environment with emergency stop capability

Test Steps:
1. - [ ] Configure CAN bus at 500 kbps bit rate using CAN 2.0B
    Expected:   CAN bus operates at 500 kbps
                Bus utilization ≤40% under normal conditions (is this possible?)
2. - [ ] Start STM32 sensor modules and verify sensor data collection
3. - [ ] Test CAN message transmission with correct IDs and periods
    Expected: CAN IDs start with 0x1... or 0x2... depending on the type of frame
            Priority frames (e.g, steer, throttle) start with 0x1...
            Non-priority (e.g, speed, battery) start with 0x2...
4. - [ ] Monitor CAN bus utilization under normal operating conditions
5. - [ ] Test heartbeat message transmission from RPi5 to STM32
    Expected:   Heartbeat transmition logs seen on PCAN or STM Logs
6. - [ ] Set gear to Park (P) mode and verify motor disabled, throttle at 0%
    Expected:   P mode: Motor control disabled, throttle locked at 0%
7. - [ ] Set gear to Neutral (N) mode and verify motor disabled, throttle at 0%
    Expected:   N mode: Motor control disabled, throttle locked at 0%
8. - [ ] Set gear to Drive (D) mode and test bidirectional motor control
    Expected:   D mode: Motor control enabled in both directions
9. - [ ] Set gear to Reverse (R) mode and verify backward-only motor control
    Expected:   R mode: Motor control enabled in backward direction only, forward commands ignored
10. - [ ] Verify gear transitions are smooth and safe
11. - [ ] Test sensor data transmission over CAN bus during all gear modes
12. - [ ] Verify timing consistency over extended periods
    Expected:   Consistent timing performance over extended operation

Postconditions:
- CAN bus communication verified as stable and within specifications
- All gear modes tested and confirmed safe
- Sensor data collection validated for accuracy and timing
- Vehicle control system ready for integration with higher-level functions

Covers:
- `dsn~can-bus-configuration~1`
- `dsn~can-message-definitions~1`
- `dsn~can-bus-load~1`
- `dsn~heartbeat-transmission~1`
- `dsn~gear-park-behavior~1`
- `dsn~gear-neutral-behavior~1`
- `dsn~gear-drive-behavior~1`
- `dsn~gear-reverse-behavior~1`

Evidence: `docs/requirements/evidence/vehicle_control_data_integration_test/`
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

## Test Case 3 - Integrated Safety and Emergency Response Test

```
`itest~safety-emergency-response-integration~1`

Status: approved

Description: Comprehensive verification of safety systems and emergency response including remote control emergency functions and safety module authority.

Preconditions:
- Remote control system paired and functional
- Safety module (STM32) programmed with safety protocols
- Vehicle in controlled test environment with emergency stop capability
- Obstacle detection system (distance sensor) operational
- Safety personnel present during all testing
- Vehicle immobilized on test stand or in secure test area

Test Steps:
2. - [ ] Test throttle control across full range (0-100%)
3. - [ ] Test steering control across full range (left-right)
4. - [ ] Test gear selection between P, N, D, R modes via remote control
    Expected:   Remote control updates throttle, steering, and gear selection with ≤100ms latency
5. - [ ] Test control responsiveness and latency for normal operations
    Expected:   Control latency ≤100ms for safety-critical functions
6. - [ ] Set up test environment with controllable obstacle detection
7. - [ ] Send normal throttle and steering commands via remote control
8. - [ ] Trigger obstacle detection within safety threshold
9. - [ ] Verify safety module activates FAILSAFE mode
    Expected:   Safety module detects obstacles and activates FAILSAFE mode
10. - [ ] Verify vehicle stops immediately (throttle 0%, safe braking activated)
    Expected:   Vehicle motion immediately stopped with throttle set to 0% and safe braking
11. - [ ] Test emergency braking activation via remote control
12. - [ ] Verify emergency braking immediately stops vehicle regardless of other inputs
13. - [ ] Verify emergency braking overrides all other commands
    Expected:   Emergency braking immediately stops vehicle regardless of other inputs
                Emergency braking via remote control overrides all other commands
14. - [ ] Verify safety override works regardless of perception module status
    Expected:   Safety module overrides perception module commands when necessary
15. - [ ] Test safety module authority during various operating scenarios
16. - [ ] Test recovery from FAILSAFE mode
    Expected:   System can safely recover from FAILSAFE mode when obstacles cleared
17. - [ ] Verify control latency ≤100ms for safety-critical functions

Postconditions:
- All safety systems verified as operational within specification
- Emergency response capabilities confirmed
- Remote control authority and limitations documented
- Vehicle certified safe for autonomous operation testing
- Safety protocols validated and ready for field testing

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