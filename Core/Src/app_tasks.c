// app_tasks.c
#include <string.h>
#include <stdio.h>
#include "stm32f103xb.h"
#include "stm32f1xx_hal.h"
#include "app_tasks.h"
#include "lcd.h"
#include "cmsis_os2.h"
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
        osMutexAcquire(lcdMutexHandle, osWaitForever);
        if (strlen((const char *)msg.data) > 0 && *msg.data == '/')
        {
            LCD_HandleCommand(&msg.data[1]);
        }
        else
        {
            LCD_Clear();
            LCD_Print((const uint8_t *)msg.data, strlen((const char *)msg.data));
        }
        osMutexRelease(lcdMutexHandle);
    }
}

#define V25_MV 1430.0f     // 25 derecedeki tipik voltaj (1.43V)
#define AVG_SLOPE 4.3f     // Tipik eğim (4.3 mV/C)
#define VREFINT_MV 1200.0f // STM32F103 tipik dahili referans voltajı (1.2V)
void App_TempSensorTask(void *argument)
{
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
            LCD_Cursor(1, 5);
            LCD_Print((uint8_t *)temp_str, (uint8_t)len);
            osMutexRelease(lcdMutexHandle);
        }

        osDelay(2000);
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