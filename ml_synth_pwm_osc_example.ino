/*
 * Copyright (c) 2021 Marcel Licence
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Dieses Programm ist Freie Software: Sie können es unter den Bedingungen
 * der GNU General Public License, wie von der Free Software Foundation,
 * Version 3 der Lizenz oder (nach Ihrer Wahl) jeder neueren
 * veröffentlichten Version, weiter verteilen und/oder modifizieren.
 *
 * Dieses Programm wird in der Hoffnung bereitgestellt, dass es nützlich sein wird, jedoch
 * OHNE JEDE GEWÄHR,; sogar ohne die implizite
 * Gewähr der MARKTFÄHIGKEIT oder EIGNUNG FÜR EINEN BESTIMMTEN ZWECK.
 * Siehe die GNU General Public License für weitere Einzelheiten.
 *
 * Sie sollten eine Kopie der GNU General Public License zusammen mit diesem
 * Programm erhalten haben. Wenn nicht, siehe <https://www.gnu.org/licenses/>.
 */

/**
 * @file ml_synth_oscillator_example.ino
 * @author Marcel Licence
 * @date 17.12.2021
 *
 * @brief   This is the main project file to test the ML_SynthLibrary (synth module)
 *          It should be compatible with ESP32 and ESP8266
 */

/*
 * pinout of ESP32 DevKit found here:
 * https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/
 */

#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#include "config.h"


#include <Arduino.h>


#ifdef ESP32
#include <Wire.h>
#endif


void blink(uint8_t cnt)
{
    delay(500);
    for (int i = 0; i < cnt; i++)
    {

        digitalWrite(LED_PIN, HIGH);
        delay(50);
        digitalWrite(LED_PIN, LOW);
        delay(200);
    }
}




void setup()
{
    /*
     * this code runs once
     */

    pinMode(LED_PIN, OUTPUT);
    blink(1);

#ifdef ARDUINO_DAISY_SEED
    DaisySeed_Setup();
#endif

    delay(500);

    Serial.begin(115200);

    Serial.println();

    Serial.printf("esp32_basic_synth  Copyright (C) 2021  Marcel Licence\n");
    Serial.printf("This program comes with ABSOLUTELY NO WARRANTY;\n");
    Serial.printf("This is free software, and you are welcome to redistribute it\n");
    Serial.printf("under certain conditions; \n");


    Delay_Init();

    Serial.printf("Initialize Synth Module\n");
    Synth_Init();

    // setup_reverb();

#ifdef BLINK_LED_PIN
    Blink_Setup();
#endif

    Serial.printf("Initialize Audio Interface\n");
    Audio_Setup();

    Serial.printf("Initialize Midi Module\n");
    /*
     * setup midi module / rx port
     */
    Midi_Setup();

#ifdef ESP32
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());
#endif

    Serial.printf("Firmware started successfully\n");

#if 1 /* activate this line to get a tone on startup to test the DAC */
    Synth_NoteOn(0, 64, 1.0f);
#endif

#if (defined ADC_TO_MIDI_ENABLED) || (defined MIDI_VIA_USB_ENABLED)
#ifdef ESP32
    Core0TaskInit();
#else
#error only supported by ESP32 platform
#endif
#endif
}

#ifdef ESP32
/*
 * Core 0
 */
/* this is used to add a task to core 0 */
TaskHandle_t Core0TaskHnd;

inline
void Core0TaskInit()
{
    /* we need a second task for the terminal output */
    xTaskCreatePinnedToCore(Core0Task, "CoreTask0", 8000, NULL, 999, &Core0TaskHnd, 0);
}

void Core0TaskSetup()
{
    /*
     * init your stuff for core0 here
     */
#ifdef ADC_TO_MIDI_ENABLED
    AdcMul_Init();
#endif

#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_Setup();
#endif
}

#ifdef ADC_TO_MIDI_ENABLED
static uint8_t adc_prescaler = 0;
#endif

void Core0TaskLoop()
{
    /*
     * put your loop stuff for core0 here
     */
#ifdef ADC_TO_MIDI_ENABLED
#ifdef MIDI_VIA_USB_ENABLED
    adc_prescaler++;
    if (adc_prescaler > 15) /* use prescaler when USB is active because it is very time consuming */
#endif /* MIDI_VIA_USB_ENABLED */
    {
        adc_prescaler = 0;
        AdcMul_Process();
    }
#endif /* ADC_TO_MIDI_ENABLED */
#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_Loop();
#endif

#ifdef MCP23_MODULE_ENABLED
    MCP23_Loop();
#endif
}

void Core0Task(void *parameter)
{
    Core0TaskSetup();

    while (true)
    {
        Core0TaskLoop();

        /* this seems necessary to trigger the watchdog */
        delay(1);
        yield();
    }
}
#endif

/*
 * use this if something should happen every second
 * - you can drive a blinking LED for example
 */
inline void Loop_1Hz(void)
{
#ifdef BLINK_LED_PIN
    Blink_Process();
#endif
}


/*
 * our main loop
 * - all is done in a blocking context
 * - do not block the loop otherwise you will get problems with your audio
 */
float fl_sample, fr_sample;

void loop()
{
    static int loop_cnt_1hz = 0; /*!< counter to allow 1Hz loop cycle */

#ifdef SAMPLE_BUFFER_SIZE
    loop_cnt_1hz += SAMPLE_BUFFER_SIZE;
#else
    loop_cnt_1hz += 1; /* in case only one sample will be processed per loop cycle */
#endif
    if (loop_cnt_1hz >= SAMPLE_RATE)
    {
        loop_cnt_1hz -= SAMPLE_RATE;
        Loop_1Hz();
    }

    /*
     * MIDI processing
     */
    //if (loop_count_u8 % 8 == 0)
    {
        Midi_Process();
#ifdef MIDI_VIA_USB_ENABLED
        UsbMidi_ProcessSync();
#endif
    }

    float left[SAMPLE_BUFFER_SIZE], right[SAMPLE_BUFFER_SIZE];

    Synth_Process(left, right, SAMPLE_BUFFER_SIZE);
    /*
     * process delay line
     */
    Delay_Process(left, right, SAMPLE_BUFFER_SIZE);

    /*
     * Output the audio
     */
    Audio_Output(left, right);
}

/*
 * MIDI via USB Host Module
 */
#ifdef MIDI_VIA_USB_ENABLED
void App_UsbMidiShortMsgReceived(uint8_t *msg)
{
    Midi_SendShortMessage(msg);
    Midi_HandleShortMsg(msg, 8);
}
#endif

/*
 * Test functions
 */
#ifdef ESP32
void  ScanI2C(void)
{

    Wire.begin(21, 22);

    byte error, address;
    int nDevices;

    Serial.println("Scanning...");

    nDevices = 0;
    for (address = 1; address < 127; address++)
    {
        // The i2c_scanner uses the return value of
        // the Write.endTransmisstion to see if
        // a device did acknowledge to the address.
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            Serial.print("I2C device found at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.print(address, HEX);
            Serial.println("  !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
            {
                Serial.print("0");
            }
            Serial.println(address, HEX);
        }
    }
    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
    }
    else
    {
        Serial.println("done\n");
    }
}
#endif
