#include "app_display.h"
#include <stdio.h>
#include <string.h>
#include "lcd.h"

void App_DisplayState(const AppState_t *state)
{
    char firstLine[17];
    char secondLine[17];

    (void)snprintf(firstLine, sizeof(firstLine), "T:%d L:%u%%",
                   state->temperature_c10 / 10, state->light_percent);
    (void)snprintf(secondLine, sizeof(secondLine), "D:%ucm A:%u",
                   state->distance_cm, state->alarm_active);

    LCD_Clear();
    LCD_Cursor(0, 0);
    LCD_Print((const uint8_t *)firstLine, strlen(firstLine));
    LCD_Cursor(1, 0);
    LCD_Print((const uint8_t *)secondLine, strlen(secondLine));
}