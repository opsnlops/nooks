#pragma once

#include <Arduino.h>
#include <ArduinoOTA.h>

extern "C"
{
#include "freertos/FreeRTOS.h"
#include "freertos/timers.h"
}

void setup_ota(String hostname);
void start_ota();

portTASK_FUNCTION_PROTO( creatureOTATask, pvParameters );
