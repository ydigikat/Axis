/*
  ------------------------------------------------------------------------------
   SynthCoreF4
   Author: ydigikat
  ------------------------------------------------------------------------------
   MIT License
   Copyright (c) 2025 YDigiKat

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
  ------------------------------------------------------------------------------
*/
#include <stdbool.h>

#include "ui.h"


/**
* FreeRTOS task that handles the user interface.
* Currently implements a simple LED blink pattern as Frugi has no UI.
* - LED on for 50ms
* - LED off for 950ms
* This creates a 1 second blink cycle with a short pulse.
*
* @param pvParameters Task parameters (unused)
*/
static void ui_task(void *pvParameters)
{
  while(1)
  {
    USR_LED_ON();
    vTaskDelay(pdMS_TO_TICKS(50));
    USR_LED_OFF();
    vTaskDelay(pdMS_TO_TICKS(950));
  }
}


/**
* Initializes and starts the UI task.
* Creates a FreeRTOS task for handling user interface operations.
*
* @param priority Priority level for the UI task
* @return true if task creation succeeded, false otherwise
* 
* Stack size: 4x minimum stack size defined by FreeRTOS config
*/
bool ui_start(UBaseType_t priority)
{
  if (xTaskCreate(ui_task, "UI", configMINIMAL_STACK_SIZE * 4, NULL, priority, NULL) != pdPASS)
  {
    return false;
  }

  return true;
}

