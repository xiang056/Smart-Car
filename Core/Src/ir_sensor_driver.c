#include "ir_sensor_driver.h"

uint8_t ir_sensor_read(void)
{
    /* LOW = obstacle detected */
    return (HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13) == GPIO_PIN_RESET) ? 1 : 0;
}
