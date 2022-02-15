
#pragma once

// Standard 8.3 (plus NUL)
#define MOVEMENT_FILE_LENGTH_MAX 13

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ESP32Servo.h>

namespace creatures
{

    void init_creature();
    void config_fail();
    bool check_file(File *file);
    struct Header read_header(File *file);
    void play_frame(File *file, size_t number_of_servos);
    void play_file(char *FILE_NAME);
    void playMovementFileTask(void *pvParamenters);

}
