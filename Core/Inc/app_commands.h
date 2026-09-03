#ifndef APP_COMMANDS_H
#define APP_COMMANDS_H

#include <stdint.h>
#include "app_state.h"

uint8_t App_CommandsProcess(AppState_t *state, const uint8_t *data,
                            uint8_t length);

#endif