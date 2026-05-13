#ifndef __IR_SENSOR_DRIVER_H
#define __IR_SENSOR_DRIVER_H

#include "stm32f4xx_hal.h"

/* IR sensor OUT: PC13, HIGH = no obstacle, LOW = obstacle */
uint8_t ir_sensor_read(void); /* returns 1 if obstacle detected, 0 if clear */

#endif /* __IR_SENSOR_DRIVER_H */
