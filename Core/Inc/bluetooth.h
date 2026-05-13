#ifndef __BLUETOOTH_H
#define __BLUETOOTH_H

#include "stm32f4xx_hal.h"

/*
 * Command protocol (single ASCII char via UART):
 *   'F' = Forward    'B' = Backward
 *   'L' = Left       'R' = Right
 *   'S' = Stop       '?' = Query status
 */
typedef enum {
    BT_CMD_NONE,
    BT_CMD_FORWARD,
    BT_CMD_BACKWARD,
    BT_CMD_LEFT,
    BT_CMD_RIGHT,
    BT_CMD_STOP,
    BT_CMD_QUERY
} BtCmd_t;

void bluetooth_init(UART_HandleTypeDef *huart);
void bluetooth_rx_callback(UART_HandleTypeDef *huart);
BtCmd_t bluetooth_get_cmd(void);
void bluetooth_rearm(void);
void bluetooth_send_status(uint16_t speed_pct, uint8_t angle, uint16_t dist_cm, uint8_t obstacle);

#endif /* __BLUETOOTH_H */
