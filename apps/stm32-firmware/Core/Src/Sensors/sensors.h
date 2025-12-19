#ifndef SENSORS_H
#define SENSORS_H

#include "main.h"
#include "tx_api.h"

/* Global Vehicle Data Structure */
typedef struct {
    float battery_voltage;      // Battery voltage in volts
    uint8_t battery_percentage; // Battery percentage (0-100%)
    float battery_current;      // Battery current in mA
    float vehicle_speed;        // Vehicle speed in km/h or m/s
    uint8_t data_valid;         // Flag: 1 if data is valid, 0 if not updated yet
} VehicleData_t;

/* Vehicle Command Structure from CAN */
typedef struct {
    uint8_t driving_mode;       // Driving mode (first byte)
    uint8_t throttle;           // Throttle value 1-100 (second byte)
    int8_t steering_angle;      // Steering angle -100 to +100 representing -1.0 to +1.0 (third byte as signed)
    uint8_t command_valid;      // Flag: 1 if command received, 0 otherwise
} VehicleCommand_t;

/* Global vehicle data accessible by all threads */
extern VehicleData_t g_vehicle_data;

/* Mutex for protecting global vehicle data */
extern TX_MUTEX g_vehicle_data_mutex;

/* Global vehicle command received via CAN */
extern VehicleCommand_t g_vehicle_command;

/* Mutex for protecting global vehicle command */
extern TX_MUTEX g_vehicle_command_mutex;

#endif
