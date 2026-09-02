// app_tasks.c
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "app_tasks.h"
#include "app_state.h"
#include "lcd.h"
#include "cmsis_os2.h"

static AppState_t appState;

static int32_t App_ParseValue(const char *command, const char *key,
                              int32_t currentValue)
{
    size_t keyLength = strlen(key);

    if (strncmp(command, key, keyLength) == 0 && command[keyLength] == '=')
    {
        char *end;
        long value = strtol(&command[keyLength + 1U], &end, 10);

        if (end != &command[keyLength + 1U] && *end == '\0')
        {
            return (int32_t)value;
        }
    }

    return currentValue;
}

static void App_ProcessSimulationCommand(const uint8_t *data, uint8_t length)
{
    char command[LCD_MSG_DATA_SIZE];
    int32_t value;

    if (length >= sizeof(command))
    {
        length = sizeof(command) - 1U;
    }
    memcpy(command, data, length);
    command[length] = '\0';

    value = App_ParseValue(command, "temp", appState.temperature_c10 / 10);
    if (strncmp(command, "temp=", 5U) == 0)
    {
        appState.temperature_c10 = (int16_t)(value * 10);
    }

    value = App_ParseValue(command, "light", appState.light_percent);
    if (strncmp(command, "light=", 6U) == 0 && value >= 0 && value <= 100)
    {
        appState.light_percent = (uint8_t)value;
    }

    value = App_ParseValue(command, "distance", appState.distance_cm);
    if (strncmp(command, "distance=", 9U) == 0 && value >= 0 && value <= UINT16_MAX)
    {
        appState.distance_cm = (uint16_t)value;
    }

    value = App_ParseValue(command, "temp_limit", appState.temperature_limit_c10 / 10);
    if (strncmp(command, "temp_limit=", 11U) == 0)
    {
        appState.temperature_limit_c10 = (int16_t)(value * 10);
    }

    value = App_ParseValue(command, "light_limit", appState.light_limit_percent);
    if (strncmp(command, "light_limit=", 12U) == 0 && value >= 0 && value <= 100)
    {
        appState.light_limit_percent = (uint8_t)value;
    }

    value = App_ParseValue(command, "distance_limit", appState.distance_limit_cm);
    if (strncmp(command, "distance_limit=", 15U) == 0 && value >= 0 && value <= UINT16_MAX)
    {
        appState.distance_limit_cm = (uint16_t)value;
    }

    if (strcmp(command, "alarm_reset") == 0)
    {
        appState.alarm_active = 0U;
    }
    else
    {
        AppState_UpdateAlarm(&appState);
    }
}

static void App_DisplayState(void)
{
    char firstLine[17];
    char secondLine[17];

    (void)snprintf(firstLine, sizeof(firstLine), "T:%d L:%u%%",
                   appState.temperature_c10 / 10, appState.light_percent);
    (void)snprintf(secondLine, sizeof(secondLine), "D:%ucm A:%u",
                   appState.distance_cm, appState.alarm_active);

    LCD_Clear();
    LCD_Cursor(0, 0);
    LCD_Print((const uint8_t *)firstLine, strlen(firstLine));
    LCD_Cursor(1, 0);
    LCD_Print((const uint8_t *)secondLine, strlen(secondLine));
}

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
            uint8_t isSimulationCommand =
                msg.length > 1U &&
                (strchr(command, '=') != NULL ||
                 strcmp(command, "alarm_reset") == 0 ||
                 strcmp(command, "status") == 0);

            if (isSimulationCommand)
            {
                App_ProcessSimulationCommand((const uint8_t *)command,
                                             msg.length - 1U);
                App_DisplayState();
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

#define V25_MV 1430.0f     // 25 derecedeki tipik voltaj (1.43V)
#define AVG_SLOPE 4.3f     // Tipik eğim (4.3 mV/C)
#define VREFINT_MV 1200.0f // STM32F103 tipik dahili referans voltajı (1.2V)
void App_TempSensorTask(void *argument)
{
    uint32_t nextWakeTime = osKernelGetTickCount();

    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adcData, 2);
    for (;;)
    {
        volatile uint16_t temp_raw = adcData[0];
        volatile uint16_t vref_raw = adcData[1];

        // STM32F103 için VREFINT (Tipik 1200mV) üzerinden gerçek VDDA hesabı
        float vdda_mv = (VREFINT_MV * 4095.0f) / (float)vref_raw;

        // Temperature sensor'ın gerçek çıkış voltajı (mV)
        float vsense_mv = ((float)temp_raw * vdda_mv) / 4095.0f;

        float temperature_c = ((V25_MV - vsense_mv) / AVG_SLOPE) + 25.0f;

        char temp_str[16];
        // Float desteği gerektirmeyen güvenli dönüşüm yöntemi
        int32_t temp_int = (int32_t)temperature_c;
        int32_t temp_frac = (int32_t)((temperature_c - (float)temp_int) * 10.0f);

        // Eğer eksi sıcaklıklarda frac kısmı negatif çıkarsa pozitife çeviriyoruz
        if (temp_frac < 0)
            temp_frac = -temp_frac;

        // Sadece tamsayı (%d) kullanarak yazdırıyoruz (Float desteği gerekmez!)
        int len = snprintf(temp_str, sizeof(temp_str), "%ld.%ld C", temp_int, temp_frac);

        if (osMutexAcquire(lcdMutexHandle, osWaitForever) == osOK)
        {
            LCD_Cursor(1, 10);
            LCD_Print((uint8_t *)temp_str, (uint8_t)len);
            osMutexRelease(lcdMutexHandle);
        }

        nextWakeTime += 2000U;
        osDelayUntil(nextWakeTime);
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