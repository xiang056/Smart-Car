#include "ultrasonic_driver.h"

static uint16_t _distance_cm = 0;

static void dwt_init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
    DWT->CYCCNT = 0;
    DWT->CTRL  |= DWT_CTRL_CYCCNTENA_Msk;
}

void ultrasonic_init(TIM_HandleTypeDef *htim)
{
    (void)htim;
    dwt_init();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef g = {0};
    g.Pin   = GPIO_PIN_2;
    g.Mode  = GPIO_MODE_INPUT;
    g.Pull  = GPIO_PULLDOWN;
    HAL_GPIO_Init(GPIOC, &g);
}

void ultrasonic_trigger(void)
{
    uint32_t c1, c2, pulse_us;
    uint16_t d;

    /* Trig 脈衝：先確保 LOW，再送 HIGH 1ms */
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);
    HAL_Delay(2);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOD, GPIO_PIN_13, GPIO_PIN_RESET);

    /* 等 Echo HIGH（逾時 30ms） */
    uint32_t t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_RESET) {
        if (HAL_GetTick() - t > 30) { _distance_cm = 0; return; }
    }
    c1 = DWT->CYCCNT;

    /* 等 Echo LOW（逾時 30ms） */
    t = HAL_GetTick();
    while (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_2) == GPIO_PIN_SET) {
        if (HAL_GetTick() - t > 30) { _distance_cm = 0; return; }
    }
    c2 = DWT->CYCCNT;

    /* cycles / (168MHz / 1MHz) = µs */
    pulse_us = (c2 - c1) / (SystemCoreClock / 1000000U);
    d = (uint16_t)(pulse_us / 58);
    _distance_cm = (d >= 2 && d <= 400) ? d : 0;
}

uint16_t ultrasonic_get_distance(void)
{
    return _distance_cm;
}

void ultrasonic_capture_callback(TIM_HandleTypeDef *htim)
{
    (void)htim;
}
