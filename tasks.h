#ifndef TASKS_H
#define TASKS_H

#include "config.h"

// System States managed by the Finite State Machine (FSM)
typedef enum {
    SYSTEM_STATE_INIT,
    SYSTEM_STATE_NORMAL,
    SYSTEM_STATE_WARNING,
    SYSTEM_STATE_ERROR
} SystemState_t;

// Structure to pass data safely across system tasks
typedef struct {
    uint8_t        current_temperature;
    SystemState_t  current_state;
    uint32_t       total_faults_logged;
} SystemTelemetry_t;

// Function Prototypes for Modular Architecture
void System_Hardware_Init(void);
void Task_Read_Sensors(SystemTelemetry_t *telemetry);
void Task_Process_FSM(SystemTelemetry_t *telemetry);
void Task_Control_Actuators(const SystemTelemetry_t *telemetry);
void Task_Telemetry_Report(const SystemTelemetry_t *telemetry);

#endif // TASKS_H
