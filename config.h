#ifndef CONFIG_H
#define CONFIG_H

#include <stdint.h>

// ESP32-S2 Peripheral Base Memory Addresses
#define ESP32_S2_GPIO_OUT_REG     (*(volatile uint32_t *)0x3f404004)
#define ESP32_S2_GPIO_ENABLE_REG  (*(volatile uint32_t *)0x3f404020)

// Hardware Pin Definitions
#define ACTUATOR_LED_PIN          15  // Connected to GPIO15
#define SYSTEM_STATUS_LED         16  // Connected to GPIO16 (Heartbeat)

// System Thresholds
#define SENSOR_CRITICAL_TEMP      35  // Celsius
#define TASK_TICK_RATE_MS         500 // Execution interval

#endif // CONFIG_H
