#ifndef __MOTOR_DRIVER_H
#define __MOTOR_DRIVER_H

#include "stm32f4xx_hal.h"

/* TIM1 ARR = 8399, APB2 84MHz, PSC=0 → 10kHz PWM */
#define MOTOR_PWM_MAX   8399

/* 轉速比例（轉向時內輪速度） */
#define MOTOR_TURN_RATIO   3   /* 內輪速 = MAX * 3/10 */

typedef enum {
    MOTOR_FORWARD,
    MOTOR_BACKWARD,
    MOTOR_STOP
} MotorDir_t;

void motor_init(TIM_HandleTypeDef *htim);

/*
 * 左右輪獨立控制
 *   left_dir / right_dir : MOTOR_FORWARD, MOTOR_BACKWARD, MOTOR_STOP
 *   left_spd / right_spd : 0 ~ MOTOR_PWM_MAX
 */
void motor_drive(MotorDir_t left_dir,  uint16_t left_spd,
                 MotorDir_t right_dir, uint16_t right_spd);

void motor_stop(void);

#endif /* __MOTOR_DRIVER_H */
