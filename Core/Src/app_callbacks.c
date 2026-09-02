#include "main.h"
#include <stdio.h>
#include <string.h>
#include "cmsis_os2.h"

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    messageQueue_t msg = {0};
    memcpy(msg.data, uartData, Size);
    msg.event_id = 1;

    // Debug: Memcpy sonrası
    printf(">>> After memcpy, msg.data[0..3]: %02X %02X %02X %02X\r\n",
           msg.data[0], msg.data[1], msg.data[2], msg.data[3]);

    // Send the received data in the queue to the LCD task for processing
    osMessageQueuePut(lcdQueueHandle, &msg, 0, 0);
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