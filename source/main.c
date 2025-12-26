/*
   MIT License
   Copyright (c) 2025 Jason Wilden

   Permission to use, copy, modify, and/or distribute this code for any purpose
   with or without fee is hereby granted, provided the above copyright notice and
   this permission notice appear in all copies.
*/

#include "FreeRTOS.h"
#include "task.h"

#include "trace.h"
#include "ui.h"
#include "dae.h"


/* Import the hardware initialisation function */
extern bool init(void);

int main(void)
{
    init();    

    if (!ui_start(tskIDLE_PRIORITY + 1))
    {
        RTT_LOG("UI task failed to start\n");
        while (1)
            ;
    }

    if(!dae_start(tskIDLE_PRIORITY + 5))
    {
        RTT_LOG("DAE task failed to start\n");
        while (1)
            ;
    }

    vTaskStartScheduler();

    /* We shouldn't get here, the RTOS scheduler must have failed to start. */
    RTT_LOG("%sFailed to start scheduler\n", RTT_CTRL_TEXT_BRIGHT_RED);
    while (1)
        ;

    return 0;
}

/* This will be called if RTOS detects that we're stomping over the stack */
void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    RTT_LOG("%sStack overflow, task:%s\n", RTT_CTRL_TEXT_BRIGHT_RED, pcTaskName);
    while (1)
        ;
}
