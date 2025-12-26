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
 * ui_tase
 * \brief the user interface thread
 * \param params unused.
 * \note this task never returns
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
 * ui_start
 * \brief creates the UI thread.
 * \param priority the priority level for the task
 * \return true if created, false otherwise
 */
bool ui_start(UBaseType_t priority)
{
  if (xTaskCreate(ui_task, "UI", configMINIMAL_STACK_SIZE * 4, NULL, priority, NULL) != pdPASS)
  {
    return false;
  }

  return true;
}

