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
 * @file config.h
 * @author Marcel Licence
 * @date 12.05.2021
 *
 * @brief This file contains the project configuration
 *
 * All definitions are visible in the entire project
 *
 * Put all your project settings here (defines, numbers, etc.)
 * configurations which are requiring knowledge of types etc.
 * shall be placed in z_config.ino (will be included at the end)
 */


#ifndef CONFIG_H_
#define CONFIG_H_


#ifdef __CDT_PARSER__
#include <cdt.h>
#endif


#ifdef TEENSYDUINO
#include <Audio.h> /* required to access teensy audio defines */
#endif


/* use following when you are using the esp32 audio kit v2.2 */
//#define ESP32_AUDIO_KIT /* project has not been tested on other hardware, modify on own risk */
//#define ES8388_ENABLED /* use this if the Audio Kit is equipped with ES8388 instead of the AC101 */

/* this will force using const velocity for all notes, remove this to get dynamic velocity */
#define MIDI_USE_CONST_VELOCITY

/* you can receive MIDI messages via serial-USB connection */
/*
 * you could use for example https://projectgus.github.io/hairless-midiserial/
 * to connect your MIDI device via computer to the serial port
 */
//#define MIDI_RECV_FROM_SERIAL

/* activate MIDI via USB */
//#define MIDI_VIA_USB_ENABLED

/*
 * Configuration for
 * Board: "LOLIN(WEMOS) D1 R2 & mini 2 or similar
 */
#ifdef ESP8266

#define SWAP_SERIAL
#define I2S_NODAC /* RX pin will be used for audio output */
#define LED_PIN     LED_BUILTIN

#define RXD2 13 /* U2RRXD, D7 */
#define TXD2 15 /* U2RRXD, D0 */
#ifndef SWAP_SERIAL
#include <SoftwareSerial.h>
SoftwareSerial Serial2(RXD2, TXD2);
#endif

#define SAMPLE_RATE 44100
#define SAMPLE_BUFFER_SIZE 48

#endif /* ESP8266 */

/*
 * Configuration for
 * Board: "ESP32 Dev Module" or similar
 */
#ifdef ESP32

#define MEMORY_FROM_HEAP

#define BOARD_ML_V1 /* activate this when using the ML PCB V1 */
//#define BOARD_ESP32_AUDIO_KIT_AC101 /* activate this when using the ESP32 Audio Kit v2.2 with the AC101 codec */
//#define BOARD_ESP32_AUDIO_KIT_ES8388 /* activate this when using the ESP32 Audio Kit v2.2 with the ES8388 codec */
//#define BOARD_ESP32_DOIT /* activate this when using the DOIT ESP32 DEVKIT V1 board */

#define LED_PIN     2
/*
 * include the board configuration
 * there you will find the most hardware depending pin settings
 */
#ifdef BOARD_ML_V1
#include "./boards/board_ml_v1.h"
#elif (defined BOARD_ESP32_AUDIO_KIT_AC101)
#include "./boards/board_audio_kit_ac101.h"
#elif (defined BOARD_ESP32_AUDIO_KIT_ES8388)
#include "./boards/board_audio_kit_es8388.h"
#elif (defined BOARD_ESP32_DOIT)
#include "./boards/board_esp32_doit.h"

#define MIDI_RX_PIN RXD2
#endif

#define SAMPLE_RATE 44100
#define SAMPLE_SIZE_16BIT
#define SAMPLE_BUFFER_SIZE  48

//#define MIDI_VIA_USB_ENABLED /* activate this when connected to the USB host breakout board */




#ifndef MIDI_SERIAL2_BAUDRATE
#define MIDI_SERIAL2_BAUDRATE 31250
#endif

#endif /* ESP32 */

/*
 * Configuration for
 * Board: "Teensy 4.1"
 *
 * BCK: 21
 * DIN: 7
 * LCK: 20
 */
#ifdef TEENSYDUINO // CORE_TEENSY

#define LED_PIN 13 /* led pin on teensy 4.1 */
#define MIDI_SERIAL1_BAUDRATE   31250
#define SAMPLE_BUFFER_SIZE AUDIO_BLOCK_SAMPLES
#define SAMPLE_RATE AUDIO_SAMPLE_RATE

#endif /* TEENSYDUINO */

/*
 * Configuration for
 * Board: "Generic STM32H7 Series"
 * Board part number: "Daisy Seed"
 */
#ifdef ARDUINO_DAISY_SEED
#define LED_PIN LED_BUILTIN
#define SAMPLE_BUFFER_SIZE  48
#define SAMPLE_RATE 48000

#define MIDI_BAUDRATE   31250
#endif

/*
 * Configuration for
 * Board: "Seeeduino XIAO"
 */
#ifdef ARDUINO_SEEED_XIAO_M0

#define LED_PIN LED_BUILTIN
#define SAMPLE_BUFFER_SIZE  48
#define SAMPLE_RATE  22050

#endif

/*
 * Configuration for
 * Board: "Rapsberry Pi Pico"
 *
 * BCK: 26
 * DIN: 28
 * LCK: 27  (always BCK + 1)
 *
 * MIDI_RX: 12 (GP9)
 *
 * Pinout @see https://www.raspberrypi-spy.co.uk/2021/01/pi-pico-pinout-and-power-pins/#prettyPhoto
 */
#ifdef ARDUINO_RASPBERRY_PI_PICO

#define LED_PIN LED_BUILTIN
#define SAMPLE_BUFFER_SIZE  48
#define SAMPLE_RATE  44100

#endif



/*
 * You can modify the sample rate as you want
 */

#define MIDI_IN RXD2
//#define MIDI_FMT_INT
#ifndef MIDI_BAUDRATE
#define MIDI_BAUDRATE   31250
#endif

#endif /* CONFIG_H_ */

