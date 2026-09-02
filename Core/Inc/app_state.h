#ifndef APP_STATE_H
#define APP_STATE_H

#include <stdint.h>

typedef struct
{
    int16_t temperature_c10;
    uint8_t light_percent;
    uint16_t distance_cm;
    int16_t temperature_limit_c10;
    uint8_t light_limit_percent;
    uint16_t distance_limit_cm;
    uint8_t alarm_active;
} AppState_t;

void AppState_Init(AppState_t *state);
void AppState_UpdateAlarm(AppState_t *state);

#endif