// app_tasks.c
#include "app_tasks.h"
#include "lcd.h"
#include "cmsis_os2.h"
#include <string.h>

void App_USARTTask(void *argument)
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart2, data, sizeof(data));
    /* Infinite loop */
    for (;;)
    {
        osDelay(1000);
    }
}

void App_LEDBlinkTask(void *argument)
{
    for (;;)
    {
        HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        osDelay(500);
    }
}

void App_LCDTask(void *argument)
{
    messageQueue_t msg = {0};
    LCD_Init();
    /* Infinite loop */
    for (;;)
    {
        osMessageQueueGet(lcdQueueHandle, &msg, NULL, osWaitForever);
        if (strlen((const char *)msg.data) > 0 && *msg.data == '/')
        {
            LCD_HandleCommand(&msg.data[1]);
        }
        else
        {
            LCD_Clear();
            LCD_Print((const uint8_t *)msg.data, strlen((const char *)msg.data));
        }
    }
}

void App_TempSensorTask(void *argument)
{
    for (;;)
    {
        osDelay(5000);
    }
}