// app_tasks.c
#include <string.h>
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "app_tasks.h"
#include "app_state.h"
#include "app_commands.h"
#include "app_display.h"
#include "lcd.h"
#include "cmsis_os2.h"

static AppState_t appState;

void App_USARTTask(void *argument)
{
    HAL_UARTEx_ReceiveToIdle_IT(&huart1, uartData, sizeof(uartData));
    /* Infinite loop */
    for (;;)
    {
        osDelay(1000);
    }
}

void App_LEDBlinkTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        uint32_t flags = osThreadFlagsWait(APP_LED_TIMER_FLAG, osFlagsWaitAny,
                                           osWaitForever);

        if ((flags & APP_LED_TIMER_FLAG) != 0U)
        {
            HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
        }
    }
}

void App_LCDTask(void *argument)
{
    messageQueue_t msg = {0};

    AppState_Init(&appState);
    LCD_Init();
    /* Infinite loop */
    for (;;)
    {
        osMessageQueueGet(lcdQueueHandle, &msg, NULL, osWaitForever);
        osMutexAcquire(lcdMutexHandle, osWaitForever);
        if (msg.length > 0U && msg.data[0] == '/')
        {
            const char *command = (const char *)&msg.data[1];
            if (msg.length > 1U)
            {
                uint8_t commandEvents = App_CommandsProcess(
                    &appState, (const uint8_t *)command, msg.length - 1U);

                if ((commandEvents & APP_COMMAND_DISTANCE_UPDATED) != 0U)
                {
                    (void)osSemaphoreRelease(DistanceReadySemaphoreHandle);
                }

                if ((commandEvents & APP_COMMAND_DISTANCE_UPDATED) == 0U &&
                    (commandEvents & APP_COMMAND_STATE_UPDATED) != 0U)
                {
                    App_DisplayState(&appState);
                }

                if ((commandEvents & APP_COMMAND_LCD_DRIVER) != 0U)
                {
                    LCD_HandleCommand(&msg.data[1]);
                }
            }
            else
            {
                LCD_HandleCommand(&msg.data[1]);
            }
        }
        else
        {
            LCD_Clear();
            LCD_Print(msg.data, msg.length);
        }
        osMutexRelease(lcdMutexHandle);
    }
}

void App_EmergencyTask(void *argument)
{
    const uint8_t emergencyMsg[] = "ACIL!";
    for (;;)
    {
        osThreadFlagsWait(0x01 /* Bit 0 = ACİL */, osFlagsWaitAny, osWaitForever);
        osMutexAcquire(lcdMutexHandle, osWaitForever);
        LCD_Clear();
        LCD_Cursor(1, 5);
        LCD_Print(emergencyMsg, strlen((const char *)emergencyMsg));
        osMutexRelease(lcdMutexHandle);
    }
}

void App_DistanceSensorTask(void *argument)
{
    (void)argument;

    for (;;)
    {
        if (osSemaphoreAcquire(DistanceReadySemaphoreHandle, 5000U) == osOK)
        {
            if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK)
            {
                AppState_UpdateAlarm(&appState);
                App_DisplayState(&appState);
                osMutexRelease(lcdMutexHandle);
            }
        }
    }
}