#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H

#include "stm32f4xx_hal.h"

/* TIM1 ARR = 8399, APB2 84MHz, PSC=0 → 10kHz PWM */
#define MOTOR_PWM_MAX   8399

/* Motor direction */
typedef enum {
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_STOP
} MotorDir_t;

void motor_init(TIM_HandleTypeDef *htim);
void motor_set(MotorDir_t dir, uint16_t speed); /* speed: 0 ~ MOTOR_PWM_MAX */
void motor_stop(void);

#endif /* __MOTOR_DRIVER_H */
