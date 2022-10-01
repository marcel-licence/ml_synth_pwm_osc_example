/*
 * Copyright (c) 2022 Marcel Licence
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
 * @see https://circuits4you.com/2018/12/31/esp32-devkit-esp32-wroom-gpio-pinout/
 */


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#include "config.h"


#include <Arduino.h>


#ifdef ESP32
#include <Wire.h>
#endif

/* requires the ml_Synth library */
#include <ml_arp.h>
#ifdef REVERB_ENABLED
#include <ml_reverb.h>
#endif
#include <ml_delay.h>
#ifdef OLED_OSC_DISP_ENABLED
#include <ml_scope.h>
#endif


void setup()
{
    /*
     * this code runs once
     */

#ifdef BLINK_LED_PIN
    Blink_Setup();
    Blink_Fast(1);
#endif

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

    Serial.printf("Initialize Synth Module\n");
    Synth_Init();

#ifdef REVERB_ENABLED
    /*
     * Initialize reverb
     * The buffer shall be static to ensure that
     * the memory will be exclusive available for the reverb module
     */
    static float revBuffer[REV_BUFF_SIZE];
    //static float *revBuffer = (float *)malloc(sizeof(float) * REV_BUFF_SIZE);
    Reverb_Setup(revBuffer);
#endif

#ifdef MAX_DELAY
    /*
     * Prepare a buffer which can be used for the delay
     */
    //static int16_t delBuffer1[MAX_DELAY];
    //static int16_t delBuffer2[MAX_DELAY];
    static int16_t *delBuffer1 = (int16_t *)malloc(sizeof(int16_t) * MAX_DELAY);
    static int16_t *delBuffer2 = (int16_t *)malloc(sizeof(int16_t) * MAX_DELAY);
    Delay_Init2(delBuffer1, delBuffer2, MAX_DELAY);
#endif

    Serial.printf("Initialize Audio Interface\n");
    Audio_Setup();

    Serial.printf("Initialize Midi Module\n");
    /*
     * setup midi module / rx port
     */
    Midi_Setup();

    Arp_Init(24 * 4); /* slowest tempo one step per bar */

#ifdef MIDI_BLE_ENABLED
    midi_ble_setup();
#endif

#ifdef ESP32
    Serial.printf("ESP.getFreeHeap() %d\n", ESP.getFreeHeap());
    Serial.printf("ESP.getMinFreeHeap() %d\n", ESP.getMinFreeHeap());
    Serial.printf("ESP.getHeapSize() %d\n", ESP.getHeapSize());
    Serial.printf("ESP.getMaxAllocHeap() %d\n", ESP.getMaxAllocHeap());
#endif

    Serial.printf("Firmware started successfully\n");

#ifdef NOTE_ON_AFTER_SETUP
    Synth_NoteOn(0, 64, 1.0f);
#endif

#ifdef MIDI_STREAM_PLAYER_ENABLED
    MidiStreamPlayer_Init();

    char midiFile[] = "/song.mid";
    MidiStreamPlayer_PlayMidiFile_fromLittleFS(midiFile, 1);
#endif

#if (defined ADC_TO_MIDI_ENABLED) || (defined MIDI_VIA_USB_ENABLED) || (defined OLED_OSC_DISP_ENABLED)
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

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Setup();
#endif

#ifdef _ADC_TO_MIDI_ENABLED
    AdcMul_Init();
#endif

#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_Setup();
#endif
}

void Core0TaskLoop()
{
    /*
     * put your loop stuff for core0 here
     */
#ifdef _ADC_TO_MIDI_ENABLED
#ifdef MIDI_VIA_USB_ENABLED
    static uint8_t adc_prescaler = 0;
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

#ifdef _MCP23_MODULE_ENABLED
    MCP23_Loop();
#endif

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_Process();
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
#endif /* ESP32 */

static uint32_t midi_sync = 0;

void Midi_SyncRecvd()
{
    midi_sync += 1;
}

void Synth_RealTimeMsg(uint8_t msg)
{
#ifndef MIDI_SYNC_MASTER
    switch (msg)
    {
    case 0xfa: /* start */
        Arp_Reset();
        break;
    case 0xf8: /* Timing Clock */
        Midi_SyncRecvd();
        break;
    }
#endif
}

#ifdef MIDI_SYNC_MASTER

#define MIDI_PPQ    24
#define SAMPLES_PER_MIN  (SAMPLE_RATE*60)

static float midi_tempo = 120.0f;

void MidiSyncMasterLoop(void)
{
    static float midiDiv = 0;
    midiDiv += SAMPLE_BUFFER_SIZE;
    if (midiDiv >= (SAMPLES_PER_MIN) / (MIDI_PPQ * midi_tempo))
    {
        midiDiv -= (SAMPLES_PER_MIN) / (MIDI_PPQ * midi_tempo);
        Midi_SyncRecvd();
    }
}

void Synth_SetMidiMasterTempo(uint8_t unused, float val)
{
    midi_tempo = 60.0f + val * (240.0f - 60.0f);
}

#endif

void Synth_SongPosition(uint16_t pos)
{
    Serial.printf("Songpos: %d\n", pos);
    if (pos == 0)
    {
        Arp_Reset();
    }
}

void Synth_SongPosReset(uint8_t unused, float var)
{
    if (var > 0)
    {
        Synth_SongPosition(0);
    }
}

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
static float left[SAMPLE_BUFFER_SIZE];
static float right[SAMPLE_BUFFER_SIZE];
#ifdef REVERB_ENABLED
static float mono[SAMPLE_BUFFER_SIZE];
#endif

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
        Loop_1Hz();
        loop_cnt_1hz = 0;
    }

#ifdef MIDI_SYNC_MASTER
    MidiSyncMasterLoop();
#endif

#ifdef ARP_MODULE_ENABLED
    Arp_Process(midi_sync);
    midi_sync = 0;
#endif

    /*
     * MIDI processing
     */
    Midi_Process();
#ifdef MIDI_VIA_USB_ENABLED
    UsbMidi_ProcessSync();
#endif
#ifdef MIDI_STREAM_PLAYER_ENABLED
#ifdef SAMPLE_BUFFER_SIZE
    MidiStreamPlayer_Tick(SAMPLE_BUFFER_SIZE);
#else
    MidiStreamPlayer_Tick(1);
#endif
#endif

#ifdef MIDI_BLE_ENABLED
    midi_ble_loop();
#endif

    /* zero buffer, otherwise you can pass trough an input signal */
    memset(left, 0, sizeof(left));
    memset(right, 0, sizeof(right));

#ifdef INPUT_TO_MIX
    Audio_Input(left, right);
#endif

    /*
     * Process synthesizer core
     */
    Synth_Process(left, right, SAMPLE_BUFFER_SIZE);

#ifdef MAX_DELAY
    /*
     * process delay line
     */
    Delay_Process_Buff2(left, right, SAMPLE_BUFFER_SIZE);
#endif

    /*
     * add some mono reverb
     */
#ifdef REVERB_ENABLED
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        mono[i] = 0.5f * (left[i] + right[i]);
    }
    Reverb_Process(mono, SAMPLE_BUFFER_SIZE);
    for (int i = 0; i < SAMPLE_BUFFER_SIZE; i++)
    {
        left[i] += mono[i];
        right[i] += mono[i];
    }
#endif

    /*
     * Output the audio
     */
    Audio_Output(left, right);

#ifdef OLED_OSC_DISP_ENABLED
    ScopeOled_AddSamples(left, right, SAMPLE_BUFFER_SIZE);
#endif
}

/*
 * Callbacks
 */
void Arp_Cb_NoteOn(uint8_t ch, uint8_t note, float vel)
{
    Synth_NoteOn(ch, note, vel);
}

void Arp_Cb_NoteOff(uint8_t ch, uint8_t note)
{
    Synth_NoteOff(ch, note);
}

void Arp_Status_ValueChangedInt(const char *msg, int value)
{
    // Status_ValueChangedInt(msg, value);
}

void Arp_Status_LogMessage(const char *msg)
{
    //Status_LogMessage(msg);
}

void Arp_Status_ValueChangedFloat(const char *msg, float value)
{
    // Status_ValueChangedFloat(msg, value);
}

void Arp_Cb_Step(uint8_t step)
{
    /* ignore */
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
#if defined(I2C_SCL) && defined (I2C_SDA) && (!defined ARDUINO_DISCO_F407VG)
void  ScanI2C(void)
{

    Wire.begin(I2C_SDA, I2C_SCL);

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

