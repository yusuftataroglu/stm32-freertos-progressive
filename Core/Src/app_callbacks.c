#include "main.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if (huart != &huart1)
    {
        return;
    }

    messageQueue_t msg = {0};
    msg.length = (Size < LCD_MSG_DATA_SIZE) ? (uint8_t)Size : LCD_MSG_DATA_SIZE - 1U;
    memcpy(msg.data, uartData, msg.length);
    msg.data[msg.length] = '\0';
    msg.event_id = 1;

    // Send the received data in the queue to the LCD task for processing
    (void)osMessageQueuePut(lcdQueueHandle, &msg, 0, 0);
    // After processing, restart the receive operation
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uartData, sizeof(uartData));
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    if (GPIO_Pin == B1_Pin)
    {
        osThreadFlagsSet(EmergencyTaskHandle, 0x01); // Bit0 set — "ACİL!"
    }
}