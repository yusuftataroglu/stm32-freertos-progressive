#include "app_state.h"

void AppState_Init(AppState_t *state)
{
    state->temperature_c10 = 250;
    state->light_percent = 50U;
    state->distance_cm = 100U;
    state->temperature_limit_c10 = 350;
    state->light_limit_percent = 20U;
    state->distance_limit_cm = 30U;
    state->alarm_active = 0U;
}

void AppState_UpdateAlarm(AppState_t *state)
{
    state->alarm_active =
        (state->temperature_c10 >= state->temperature_limit_c10) ||
        (state->light_percent <= state->light_limit_percent) ||
        (state->distance_cm <= state->distance_limit_cm);
}