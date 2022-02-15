

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <ESP32Servo.h>

#include "creatures/creatures.h"

#include "movement.h"
#include "creature.h"

namespace creatures
{

    // We need to use ADC1. ADC2 is used by the Wifi. (Pins GPIO32-GPIO39)
    Servo servos[2];
    const int servo0Pin = 13;
    const int servo1Pin = 14;

    QueueHandle_t movementQueue;

    uint8_t MAGIC_NUMBER_ARRAY[5] = {0x52, 0x41, 0x57, 0x52, 0x21};

    void init_creature()
    {
        ESP32PWM::allocateTimer(0);
        ESP32PWM::allocateTimer(1);
        ESP32PWM::allocateTimer(2);
        ESP32PWM::allocateTimer(3);

        Serial.println("attaching to servo 0");
        servos[0].setPeriodHertz(50);
        servos[0].attach(servo0Pin);
        Serial.println("done");

        Serial.println("attaching to servo 1");
        servos[1].setPeriodHertz(50);
        servos[1].attach(servo1Pin);
        Serial.println("done");
    }

    void config_fail()
    {
        while (true)
        {
            digitalWrite(LED_BUILTIN, HIGH); // turn the LED on (HIGH is the voltage level)
            delay(100);                      // wait for a second
            digitalWrite(LED_BUILTIN, LOW);  // turn the LED off by making the voltage LOW
            delay(100);                      // wait for a second
        }
    }

    bool check_file(File *file)
    {

        // Our magic number is 5 bytes long
        const uint8_t magic_number_size = 5;

        bool return_code = false;

        // Make sure the first five bytes are our magic number
        char *buffer = (char *)malloc(sizeof(uint8_t) * magic_number_size);

        file->readBytes(buffer, magic_number_size);

        for (int i = 0; i < magic_number_size; i++)
        {
            if (buffer[i] != MAGIC_NUMBER_ARRAY[i])
            {
                Serial.print("Magic Number fail at position ");
                Serial.println(i);
                return_code = false;
            }
        }

        // If we made it this far, we're good
        return_code = true;

        free(buffer);

        return return_code;
    }

    // Returns the header from the file
    struct Header read_header(File *file)
    {
        struct Header header;

        file->readBytes((char *)&header, sizeof(Header));

        Serial.print("number of servos: ");
        Serial.print(header.number_of_servos);
        Serial.print(", number of frames: ");
        Serial.print(header.number_of_frames);
        Serial.print(", ms per frame: ");
        Serial.println(header.time_per_frame);

        return header;
    }

    void play_frame(File *file, size_t number_of_servos)
    {
        uint8_t servo[number_of_servos];
        file->readBytes((char *)&servo, number_of_servos);
        for (int i = 0; i < number_of_servos; i++)
        {

#ifdef CREATURE_DEBUG
            Serial.println(servo[i]);
#endif

            servos[i].write(servo[i]);
        }
    }

    void play_file(char *FILE_NAME)
    {

        uint16_t currentFrame = 0;

        // open the file for reading:
        File myFile = SD.open(FILE_NAME);
        if (myFile)
        {
            Serial.println(FILE_NAME);

            if (!check_file(&myFile))
            {
                config_fail();
            }

            struct Header header = read_header(&myFile);

            // read from the file until there's nothing else in it:
            while (myFile.available())
            {

                uint8_t command = myFile.read();
                if (command == (uint8_t)MOVEMENT_FRAME_TYPE)
                {
                    play_frame(&myFile, header.number_of_servos);
                    vTaskDelay(TickType_t pdMS_TO_TICKS(header.time_per_frame));
                }

                if (currentFrame % 10 == 0)
                {
                    Serial.print("frame ");
                    Serial.println(currentFrame);
                }

                currentFrame++;
            }
            // close the file:
            myFile.close();
        }
        else
        {
            // if the file didn't open, print an error:
            Serial.print("error opening ");
            Serial.println(FILE_NAME);
            config_fail();
        }
    }

    void playMovementFileTask(void *pvParamenters)
    {

        char fileName[MOVEMENT_FILE_LENGTH_MAX];
        memset(fileName, '\0', MOVEMENT_FILE_LENGTH_MAX);

        for (;;)
        {
            if (movementQueue != NULL)
            {
                if (xQueueReceive(movementQueue, &fileName, (TickType_t)150) == pdPASS)
                {
                    Serial.printf("request to play %s received\n", fileName);
                    play_file(fileName);
                }
            }
        }
    }

}