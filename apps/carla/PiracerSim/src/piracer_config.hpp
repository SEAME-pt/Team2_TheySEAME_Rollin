#pragma once

namespace piracer {

// Car physical parameters
constexpr double WHEELBASE_M = 0.207; // Wheelbase length in meters
constexpr double MASS_KG = 3.2;      // Car mass in kilograms
constexpr double MAX_STEER_DEG = 30.0; // Maximum steering angle in degrees
constexpr double MAX_SPEED_MPS = 14;  // Maximum speed in meters per second

// Sensor update rates
constexpr double WORLD_DT_S = 0.05;     // Simulation timestep (20 Hz)
constexpr double CAM_TICK_S = 0.05;     // Camera update interval (20 Hz)
constexpr double IMU_TICK_S = 0.01;     // IMU update interval (100 Hz)
constexpr double GNSS_TICK_S = 0.05;    // GNSS update interval (20 Hz)

// Camera mounting position and orientation
constexpr double CAM_X = 0.25;          // Camera X position (meters)
constexpr double CAM_Y = 0.0;           // Camera Y position (meters)
constexpr double CAM_Z = 0.18;          // Camera Z position (meters)
constexpr double CAM_PITCH_DEG = -5.0;  // Camera pitch angle (degrees)

// Control parameters
constexpr double CONTROL_LATENCY_S = 0.08; // Simulated control latency (seconds)
constexpr double MAX_STEER_SLEW_PER_TICK = 0.08;   // Max steering change per tick
constexpr double MAX_THROTTLE_SLEW_PER_TICK = 0.10; // Max throttle change per tick

} // namespace piracer