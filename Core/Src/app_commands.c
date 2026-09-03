#include "app_commands.h"
#include <stdlib.h>
#include <string.h>
#include "main.h"

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

uint8_t App_CommandsProcess(AppState_t *state, const uint8_t *data,
                            uint8_t length)
{
    char command[LCD_MSG_DATA_SIZE];
    int32_t value;

    if (length >= sizeof(command))
    {
        length = sizeof(command) - 1U;
    }
    memcpy(command, data, length);
    command[length] = '\0';

    if (strcmp(command, "status") == 0)
    {
        return 1U;
    }

    value = App_ParseValue(command, "temp", state->temperature_c10 / 10);
    if (strncmp(command, "temp=", 5U) == 0)
    {
        state->temperature_c10 = (int16_t)(value * 10);
    }

    value = App_ParseValue(command, "light", state->light_percent);
    if (strncmp(command, "light=", 6U) == 0 && value >= 0 && value <= 100)
    {
        state->light_percent = (uint8_t)value;
    }

    value = App_ParseValue(command, "distance", state->distance_cm);
    if (strncmp(command, "distance=", 9U) == 0 && value >= 0 && value <= UINT16_MAX)
    {
        state->distance_cm = (uint16_t)value;
    }

    value = App_ParseValue(command, "temp_limit", state->temperature_limit_c10 / 10);
    if (strncmp(command, "temp_limit=", 11U) == 0)
    {
        state->temperature_limit_c10 = (int16_t)(value * 10);
    }

    value = App_ParseValue(command, "light_limit", state->light_limit_percent);
    if (strncmp(command, "light_limit=", 12U) == 0 && value >= 0 && value <= 100)
    {
        state->light_limit_percent = (uint8_t)value;
    }

    value = App_ParseValue(command, "distance_limit", state->distance_limit_cm);
    if (strncmp(command, "distance_limit=", 15U) == 0 && value >= 0 && value <= UINT16_MAX)
    {
        state->distance_limit_cm = (uint16_t)value;
    }

    if (strcmp(command, "alarm_reset") == 0)
    {
        state->alarm_active = 0U;
    }
    else
    {
        AppState_UpdateAlarm(state);
    }

    return (strchr(command, '=') != NULL || strcmp(command, "alarm_reset") == 0);
}