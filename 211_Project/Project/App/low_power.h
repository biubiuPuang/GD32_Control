#ifndef LOW_POWER_H
#define LOW_POWER_H

#include "gd32e23x.h"
#include "Debug_printf.h"

void low_power_init(void);
void low_power_note_activity(void);
void low_power_process(void);

void low_power_timer13_irq_handler(void);
void low_power_key_exti_handler(void);

#endif

