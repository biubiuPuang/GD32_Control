#ifndef LOW_POWER_CONFIG_H
#define LOW_POWER_CONFIG_H

#include <stdint.h>

#define LOW_POWER_TIMEOUT_SECONDS 300U

extern volatile uint32_t inactivity_seconds;
extern volatile uint8_t sleep_request;
extern volatile uint8_t low_power_mode;

#endif
