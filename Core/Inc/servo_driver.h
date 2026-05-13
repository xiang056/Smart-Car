#ifndef __SERVO_DRIVER_H
#define __SERVO_DRIVER_H

#include "stm32f4xx_hal.h"

/*
 * TIM3 CH1 (PA6), APB1_TIM=84MHz, PSC=839, ARR=1999
 * Each tick = 840/84MHz = 10us, Period = 2000*10us = 20ms (50Hz)
 * 0°  → CCR = 50  (0.5ms)
 * 90° → CCR = 150 (1.5ms)
 * 180°→ CCR = 250 (2.5ms)
 */
#define SERVO_CCR_MIN   50
#define SERVO_CCR_MID   150
#define SERVO_CCR_MAX   250

void servo_init(TIM_HandleTypeDef *htim);
void servo_set_angle(uint8_t angle); /* angle: 0 ~ 180 */

#endif /* __SERVO_DRIVER_H */
