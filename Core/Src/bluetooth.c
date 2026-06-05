#include "bluetooth.h"
#include <stdio.h>
#include <string.h>

static UART_HandleTypeDef *_huart;
static volatile uint8_t _rx_byte;
static volatile BtCmd_t _pending_cmd = BT_CMD_NONE;

void bluetooth_init(UART_HandleTypeDef *huart)
{
    _huart = huart;
    HAL_UART_Receive_IT(huart, (uint8_t *)&_rx_byte, 1);
}

void bluetooth_rx_callback(UART_HandleTypeDef *huart)
{
    if (huart->Instance != _huart->Instance)
        return;

    switch (_rx_byte) {
        case 'F': case 'f': _pending_cmd = BT_CMD_FORWARD;  break;
        case 'B': case 'b': _pending_cmd = BT_CMD_BACKWARD; break;
        case 'L': case 'l': _pending_cmd = BT_CMD_LEFT;     break;
        case 'R': case 'r': _pending_cmd = BT_CMD_RIGHT;    break;
        case 'S': case 's': _pending_cmd = BT_CMD_STOP;     break;
        case '?':           _pending_cmd = BT_CMD_QUERY;    break;
        default: break;
    }

    HAL_UART_Receive_IT(huart, (uint8_t *)&_rx_byte, 1);
}

BtCmd_t bluetooth_get_cmd(void)
{
    __disable_irq();
    BtCmd_t cmd = _pending_cmd;
    _pending_cmd = BT_CMD_NONE;
    __enable_irq();
    return cmd;
}

void bluetooth_rearm(void)
{
    if (_huart == NULL) return;
    /* AbortReceive 正確清除 HAL 狀態機並禁用中斷，再重新武裝 */
    HAL_UART_AbortReceive(_huart);
    __HAL_UART_CLEAR_OREFLAG(_huart);
    HAL_UART_Receive_IT(_huart, (uint8_t *)&_rx_byte, 1);
}

void bluetooth_send_status(uint16_t speed_pct, uint8_t state)
{
    char buf[16];
    /* 格式 "S,100,4\n" = 8 bytes，遠低於 HM-10 BLE MTU 20 bytes */
    int len = snprintf(buf, sizeof(buf), "S,%u,%u\n", speed_pct, state);
    if (len > 0)
        HAL_UART_Transmit(_huart, (uint8_t *)buf, (uint16_t)len, 50);
}
