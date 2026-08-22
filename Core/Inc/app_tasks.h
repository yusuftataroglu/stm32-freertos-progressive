#ifndef APP_TASKS_H
#define APP_TASKS_H
#include "main.h"
void App_LEDBlinkTask(void *argument);
void App_USARTTask(void *argument);
void App_LCDTask(void *argument);
void App_TempSensorTask(void *argument);
#endif