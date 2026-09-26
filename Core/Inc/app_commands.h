#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H

#include <stdint.h>
#include "app_state.h"

#define APP_COMMAND_DISTANCE_UPDATED 0x01U
#define APP_COMMAND_STATE_UPDATED 0x02U
#define APP_COMMAND_LCD_DRIVER 0x04U

uint8_t App_CommandsProcess(AppState_t *state, const uint8_t *data,
                            uint8_t length);

#endif