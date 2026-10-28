#include "config.h"
#include "tasks.h"

// Global Instantiation of System Telemetry (The Single Source of Truth)
static SystemTelemetry_t Global_Telemetry = {0, SYSTEM_STATE_INIT, 0};

void setup() {
    System_Hardware_Init();
    Global_Telemetry.current_state = SYSTEM_STATE_NORMAL;
}

void loop() {
    // Non-blocking Task Scheduler using a software tick
    static uint32_t last_task_tick = 0;
    uint32_t current_time = millis();

    if (current_time - last_task_tick >= TASK_TICK_RATE_MS) {
        last_task_tick = current_time;

        // Execute Industrial Firmware Pipeline
        Task_Read_Sensors(&Global_Telemetry);
        Task_Process_FSM(&Global_Telemetry);
        Task_Control_Actuators(&Global_Telemetry);
        Task_Telemetry_Report(&Global_Telemetry);
    }
}

void System_Hardware_Init(void) {
    Serial.begin(115200);
    while(!Serial); // Wait for USB connection on ESP32-S2
    
    // Configure Pin 15 and Pin 16 as Outputs using a single bitwise OR operation
    ESP32_S2_GPIO_ENABLE_REG |= (1 << ACTUATOR_LED_PIN) | (1 << SYSTEM_STATUS_LED);
    
    Serial.println("[SYS_INIT] 32-Bit registers configured. Task Scheduler online.");
}

void Task_Read_Sensors(SystemTelemetry_t *telemetry) {
    // Simulates an ADC read from a temperature sensor (flutters around thresholds)
    static uint8_t raw_sensor_feed = 27;
    static bool ramp_up = true;

    if (ramp_up) {
        raw_sensor_feed += 2;
        if (raw_sensor_feed >= 39) ramp_up = false;
    } else {
        raw_sensor_feed -= 3;
        if (raw_sensor_feed <= 25) ramp_up = true;
    }
    telemetry->current_temperature = raw_sensor_feed;
}

void Task_Process_FSM(SystemTelemetry_t *telemetry) {
    // Finite State Machine (FSM) Engine
    switch (telemetry->current_state) {
        case SYSTEM_STATE_NORMAL:
            if (telemetry->current_temperature >= SENSOR_CRITICAL_TEMP) {
                telemetry->current_state = SYSTEM_STATE_WARNING;
                telemetry->total_faults_logged++;
            }
            break;

        case SYSTEM_STATE_WARNING:
            if (telemetry->current_temperature < SENSOR_CRITICAL_TEMP) {
                telemetry->current_state = SYSTEM_STATE_NORMAL;
            } else if (telemetry->current_temperature >= 38) {
                telemetry->current_state = SYSTEM_STATE_ERROR;
            }
            break;

        case SYSTEM_STATE_ERROR:
            // Safety fallback: only clear error if conditions drop significantly low
            if (telemetry->current_temperature <= 30) {
                telemetry->current_state = SYSTEM_STATE_NORMAL;
            }
            break;

        default:
            telemetry->current_state = SYSTEM_STATE_NORMAL;
            break;
    }
}

void Task_Control_Actuators(const SystemTelemetry_t *telemetry) {
    // Toggles the System Heartbeat LED on every single loop execution to prove system liveness
    ESP32_S2_GPIO_OUT_REG ^= (1 << SYSTEM_STATUS_LED);

    // Apply hardware overrides strictly mapped to safety states
    if (telemetry->current_state == SYSTEM_STATE_WARNING || telemetry->current_state == SYSTEM_STATE_ERROR) {
        ESP32_S2_GPIO_OUT_REG |= (1 << ACTUATOR_LED_PIN); // Turn Actuator ON
    } else {
        ESP32_S2_GPIO_OUT_REG &= ~(1 << ACTUATOR_LED_PIN); // Turn Actuator OFF
    }
}

void Task_Telemetry_Report(const SystemTelemetry_t *telemetry) {
    // Standard structured serialization format for debugging or external UART reading
    Serial.print("[TELEMETRY] Temp: ");
    Serial.print(telemetry->current_temperature);
    Serial.print(" C | State: ");
    
    switch(telemetry->current_state) {
        case SYSTEM_STATE_NORMAL:  Serial.print("NORMAL");  break;
        case SYSTEM_STATE_WARNING: Serial.print("WARNING"); break;
        case SYSTEM_STATE_ERROR:   Serial.print("ERROR");   break;
        default:                   Serial.print("UNKNOWN"); break;
    }
    
    Serial.print(" | Fault Count: ");
    Serial.println(telemetry->total_faults_logged);
}
