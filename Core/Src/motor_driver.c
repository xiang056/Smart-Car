#include "motor_driver.h"

/*
 * Hardware mapping:
 *   Motor A (前): IN1=PE5, IN2=PE6, EN=PE9 (TIM1_CH1)
 *   Motor B (後): IN3=PE11, IN4=PE13, EN=PE14 (TIM1_CH4)
 */

static TIM_HandleTypeDef *_htim;

void motor_init(TIM_HandleTypeDef *htim)
{
    _htim = htim;
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_4);
    motor_stop();
}

void motor_set(MotorDir_t dir, uint16_t speed)
{
    if (speed > MOTOR_PWM_MAX) speed = MOTOR_PWM_MAX;

    switch (dir) {
        case MOTOR_FORWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5,  GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_RESET);
            break;
        case MOTOR_BACKWARD:
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5,  GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_6,  GPIO_PIN_SET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_11, GPIO_PIN_RESET);
            HAL_GPIO_WritePin(GPIOE, GPIO_PIN_13, GPIO_PIN_SET);
            break;
        default:
            motor_stop();
            return;
    }

    /* 強制重啟 TIM1：CEN/CC1E/CC4E/MOE 全部確保有效 */
    _htim->Instance->CR1  |= 0x0001U;             /* CEN  */
    _htim->Instance->CCER |= (0x0001U | 0x1000U); /* CC1E | CC4E */
    _htim->Instance->BDTR |= 0x8000U;             /* MOE */
    _htim->Instance->CCR1  = speed;
    _htim->Instance->CCR4  = speed;
}

void motor_stop(void)
{
    HAL_GPIO_WritePin(GPIOE, GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_11 | GPIO_PIN_13, GPIO_PIN_RESET);
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, 0);
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_4, 0);
}
