#include "servo_driver.h"

static TIM_HandleTypeDef *_htim;

void servo_init(TIM_HandleTypeDef *htim)
{
    _htim = htim;
    HAL_TIM_PWM_Start(htim, TIM_CHANNEL_1);
    servo_set_angle(90);
}

void servo_set_angle(uint8_t angle)
{
    if (angle > 180) angle = 180;
    uint32_t ccr = SERVO_CCR_MIN + ((uint32_t)angle * (SERVO_CCR_MAX - SERVO_CCR_MIN)) / 180;
    __HAL_TIM_SET_COMPARE(_htim, TIM_CHANNEL_1, ccr);
}
