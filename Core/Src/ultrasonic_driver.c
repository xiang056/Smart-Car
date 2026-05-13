#include "ultrasonic_driver.h"

static TIM_HandleTypeDef *_htim;
static uint16_t _distance_cm = 0;

void ultrasonic_init(TIM_HandleTypeDef *htim)
{
    _htim = htim;
    HAL_TIM_Base_Start(htim);

    /* MX_TIM2_Init 把 PA2 設為 TIM2_CH3 AF，必須改回普通 GPIO 輸入
       加上下拉確保 Echo 未拉高時讀到 LOW */
    GPIO_InitTypeDef g = {0};
    g.Pin   = GPIO_PIN_2;
    g.Mode  = GPIO_MODE_INPUT;
    g.Pull  = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOA, &g);
}

void ultrasonic_trigger(void)
{
    uint32_t t, t1, t2, pulse_us;
    uint16_t d;

    /* Trig 脈衝 10µs */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    t = __HAL_TIM_GET_COUNTER(_htim);
    while (__HAL_TIM_GET_COUNTER(_htim) - t < 10);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

    /* 等 Echo 變 HIGH，逾時 30ms */
    t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - t > 30) { _distance_cm = 0; return; }
    }
    t1 = __HAL_TIM_GET_COUNTER(_htim);

    /* 等 Echo 變 LOW，逾時 30ms */
    t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_2) == GPIO_PIN_SET) {
        if (HAL_GetTick() - t > 30) { _distance_cm = 0; return; }
    }
    t2 = __HAL_TIM_GET_COUNTER(_htim);

    pulse_us = (t2 >= t1) ? (t2 - t1) : (0xFFFFFFFF - t1 + t2);
    d = (uint16_t)(pulse_us / 58);
    _distance_cm = (d >= 2 && d <= 300) ? d : 0;
}

uint16_t ultrasonic_get_distance(void)
{
    return _distance_cm;
}

void ultrasonic_capture_callback(TIM_HandleTypeDef *htim)
{
    (void)htim;
}
