#include "motor_driver.h"

/*
 * Hardware mapping:
 *   左輪 (Motor A): IN1=PE5, IN2=PE6, EN=PE9  (TIM1_CH1 → CCR1)
 *   右輪 (Motor B): IN3=PE11, IN4=PE13, EN=PE14 (TIM1_CH4 → CCR4)
 */

static TIM_HandleTypeDef *_htim;

void motor_init(TIM_HandleTypeDef *htim)
{
    _htim = htim;
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4);
    motor_stop();
}

void motor_drive(MotorDir_t left_dir,  uint16_t left_spd,
                 MotorDir_t right_dir, uint16_t right_spd)
{
    if (left_spd  > MOTOR_PWM_MAX) left_spd  = MOTOR_PWM_MAX;
    if (right_spd > MOTOR_PWM_MAX) right_spd = MOTOR_PWM_MAX;

    /* 左輪：PE5=IN1, PE6=IN2 */
    switch (left_dir) {
        case MOTOR_FORWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_SET);
            break;
        case MOTOR_BACKWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6, GPIO_PIN_RESET);
            break;
        default:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5 | GPIO_PIN_6, GPIO_PIN_RESET);
            left_spd = 0;
            break;
    }

    /* 右輪：PE11=IN3, PE13=IN4 */
    switch (right_dir) {
        case MOTOR_FORWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
            break;
        case MOTOR_BACKWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
            break;
        default:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11 | GPIO_PIN_13, GPIO_PIN_RESET);
            right_spd = 0;
            break;
    }

    /* TIM1 強制使能（進階計時器需要 MOE / CC1E / CC4E / CEN） */
    _htim->Instance->CR1  |= 0x0001U;
    _htim->Instance->CCER |= (0x0001U | 0x1000U);
    _htim->Instance->BDTR |= 0x8000U;
    _htim->Instance->CCR1  = left_spd;
    _htim->Instance->CCR4  = right_spd;
}

void motor_stop(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_11 | GPIO_PIN_13,
                      GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_4, 0);
}
