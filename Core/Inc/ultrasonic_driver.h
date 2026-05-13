#ifndef __ULTRASONIC_DRIVER_H
#define __ULTRASONIC_DRIVER_H

#include "stm32f4xx_hal.h"

/*
 * Trig: PD13 (GPIO Output, ≥10µs pulse)
 * Echo: PA2  (TIM2_CH3 Input Capture, BOTHEDGE)
 * TIM2: PSC=83, APB1_TIM=84MHz → 1µs per tick
 */

void ultrasonic_init(TIM_HandleTypeDef *htim);
void ultrasonic_trigger(void);          /* send 10µs Trig pulse */
uint16_t ultrasonic_get_distance(void); /* returns last measured cm */

/* Called from HAL_TIM_IC_CaptureCallback */
void ultrasonic_capture_callback(TIM_HandleTypeDef *htim);

#endif /* __ULTRASONIC_DRIVER_H */
