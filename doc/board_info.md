<h1>Board Build Variants</h1>

Below you will find a list of build which can be configured and should compile without any problems

<hr>
<a name="STMicroelectronics_stm32_Disco"></a><b>Core:</b> <a h_ref="https://github.com/stm32duino/Arduino_Core_STM32">STM32 boards groups </a><br />
<b>Version:</b> 2.6.0<br />
<b>Board:</b> Discovery<br />
<br />
<b>Program storage space:</b> 66668 bytes<br />
<b>Dynamic memory:</b> 26048 bytes<br />
<br />
<b>Board part number:</b> STM32F407G-DISC1<br />
<b>Upload method:</b> STM32CubeProgrammer (SWD)<br />
<b>U(S)ART support:</b> Enabled (generic 'Serial')<br />
<b>USB support (if available):</b> CDC (generic 'Serial' supersede U(S)ART)<br />
<b>USB speed (if available):</b> Low/Full Speed<br />
<b>Optimize:</b> Smallest (-Os default)<br />
<b>Debug symbols and core logs:</b> None<br />
<b>C Runtime Library:</b> Newlib Nano (default)<br />
<br />
<b>Used libraries:</b><br />
<table>
    <tr>
        <td>Name</td>
        <td>Version</td>
        <td>Url</td>
        <td>Git</td>
        <td>Core library</td>
    </tr>
    <tr>
        <td>ML SynthTools</td>
        <td>1.3.1</td>
        <td>https://github.com/marcel-licence/ML_SynthTools</td>
        <td>https://github.com/marcel-licence/ML_SynthTools.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>SrcWrapper</td>
        <td>1.0.1</td>
        <td>https://github.com/stm32duino/Arduino_Core_STM32</td>
        <td></td>
        <td>True</td>
    </tr>
</table><hr>
<a name="STMicroelectronics_stm32_GenH7"></a><b>Core:</b> <a h_ref="https://github.com/stm32duino/Arduino_Core_STM32">STM32 boards groups </a><br />
<b>Version:</b> 2.6.0<br />
<b>Board:</b> Generic STM32H7 Series<br />
<br />
<b>Program storage space:</b> 93156 bytes<br />
<b>Dynamic memory:</b> 28824 bytes<br />
<br />
<b>Board part number:</b> Daisy Seed<br />
<b>Upload method:</b> STM32CubeProgrammer (SWD)<br />
<b>U(S)ART support:</b> Enabled (generic 'Serial')<br />
<b>USB support (if available):</b> CDC (generic 'Serial' supersede U(S)ART)<br />
<b>USB speed (if available):</b> Low/Full Speed<br />
<b>Optimize:</b> Smallest (-Os default)<br />
<b>Debug symbols and core logs:</b> None<br />
<b>C Runtime Library:</b> Newlib Nano (default)<br />
<br />
<b>Used libraries:</b><br />
<table>
    <tr>
        <td>Name</td>
        <td>Version</td>
        <td>Url</td>
        <td>Git</td>
        <td>Core library</td>
    </tr>
    <tr>
        <td>ML SynthTools</td>
        <td>1.3.1</td>
        <td>https://github.com/marcel-licence/ML_SynthTools</td>
        <td>https://github.com/marcel-licence/ML_SynthTools.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>DaisyDuino</td>
        <td>1.6.3</td>
        <td>https://github.com/electro-smith/DaisyDuino</td>
        <td>https://github.com/electro-smith/DaisyDuino.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Wire</td>
        <td>1.0.0</td>
        <td>http://www.arduino.cc/en/Reference/Wire</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>SrcWrapper</td>
        <td>1.0.1</td>
        <td>https://github.com/stm32duino/Arduino_Core_STM32</td>
        <td></td>
        <td>True</td>
    </tr>
</table><hr>
<a name="espressif_2.0.5_esp32_d1_mini32"></a><b>Core:</b> <a h_ref="https://github.com/espressif/arduino-esp32">ESP32 Arduino </a><br />
<b>Version:</b> 2.0.5<br />
<b>Board:</b> WEMOS D1 MINI ESP32<br />
<br />
<b>Program storage space:</b> 758253 bytes<br />
<b>Dynamic memory:</b> 60032 bytes<br />
<br />
<b>Flash Frequency:</b> 80MHz<br />
<b>Partition Scheme:</b> Default<br />
<b>CPU Frequency:</b> 240MHz (WiFi/BT)<br />
<b>Upload Speed:</b> 921600<br />
<b>Core Debug Level:</b> None<br />
<br />
<b>Used libraries:</b><br />
<table>
    <tr>
        <td>Name</td>
        <td>Version</td>
        <td>Url</td>
        <td>Git</td>
        <td>Core library</td>
    </tr>
    <tr>
        <td>ML SynthTools</td>
        <td>1.3.1</td>
        <td>https://github.com/marcel-licence/ML_SynthTools</td>
        <td>https://github.com/marcel-licence/ML_SynthTools.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Wire</td>
        <td>2.0.0</td>
        <td>http://arduino.cc/en/Reference/Wire</td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>WiFi</td>
        <td>2.0.0</td>
        <td></td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>FS</td>
        <td>2.0.0</td>
        <td></td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>LittleFS</td>
        <td>2.0.0</td>
        <td></td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>SD_MMC</td>
        <td>2.0.0</td>
        <td></td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>Adafruit GFX Library</td>
        <td>1.11.5</td>
        <td>https://github.com/adafruit/Adafruit-GFX-Library</td>
        <td>https://github.com/adafruit/Adafruit-GFX-Library</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Adafruit BusIO</td>
        <td>1.14.1</td>
        <td>https://github.com/adafruit/Adafruit_BusIO</td>
        <td>https://github.com/adafruit/Adafruit_BusIO</td>
        <td>False</td>
    </tr>
    <tr>
        <td>SPI</td>
        <td>2.0.0</td>
        <td>http://arduino.cc/en/Reference/SPI</td>
        <td>https://github.com/espressif/arduino-esp32.git</td>
        <td>True</td>
    </tr>
    <tr>
        <td>Adafruit SSD1306</td>
        <td>2.5.7</td>
        <td>https://github.com/adafruit/Adafruit_SSD1306</td>
        <td>https://github.com/adafruit/Adafruit_SSD1306</td>
        <td>False</td>
    </tr>
</table><hr>
<a name="esp32_esp32_esp32"></a><b>Core:</b> <a h_ref="N/A">ESP32 Arduino</a><br />
<b>Version:</b> 1.0.6<br />
<b>Board:</b> ESP32 Dev Module<br />
<br />
<b>Program storage space:</b> 714006 bytes<br />
<b>Dynamic memory:</b> 60812 bytes<br />
<br />
<b>PSRAM:</b> Disabled<br />
<b>Partition Scheme:</b> Default 4MB with spiffs (1.2MB APP/1.5MB SPIFFS)<br />
<b>CPU Frequency:</b> 240MHz (WiFi/BT)<br />
<b>Flash Mode:</b> QIO<br />
<b>Flash Frequency:</b> 80MHz<br />
<b>Flash Size:</b> 4MB (32Mb)<br />
<b>Upload Speed:</b> 921600<br />
<b>Core Debug Level:</b> None<br />
<br />
<b>Used libraries:</b><br />
<table>
    <tr>
        <td>Name</td>
        <td>Version</td>
        <td>Url</td>
        <td>Git</td>
        <td>Core library</td>
    </tr>
    <tr>
        <td>ML SynthTools</td>
        <td>1.3.1</td>
        <td>https://github.com/marcel-licence/ML_SynthTools</td>
        <td>https://github.com/marcel-licence/ML_SynthTools.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Wire</td>
        <td>1.0.1</td>
        <td>http://arduino.cc/en/Reference/Wire</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>WiFi</td>
        <td>1.0</td>
        <td></td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>FS</td>
        <td>1.0</td>
        <td></td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>LittleFS_esp32</td>
        <td>1.0.6</td>
        <td>https://github.com/lorol/LITTLEFS</td>
        <td>https://github.com/lorol/LITTLEFS</td>
        <td>False</td>
    </tr>
    <tr>
        <td>SD_MMC</td>
        <td>1.0</td>
        <td></td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>Adafruit GFX Library</td>
        <td>1.11.5</td>
        <td>https://github.com/adafruit/Adafruit-GFX-Library</td>
        <td>https://github.com/adafruit/Adafruit-GFX-Library</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Adafruit BusIO</td>
        <td>1.14.1</td>
        <td>https://github.com/adafruit/Adafruit_BusIO</td>
        <td>https://github.com/adafruit/Adafruit_BusIO</td>
        <td>False</td>
    </tr>
    <tr>
        <td>SPI</td>
        <td>1.0</td>
        <td>http://arduino.cc/en/Reference/SPI</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>Adafruit SSD1306</td>
        <td>2.5.7</td>
        <td>https://github.com/adafruit/Adafruit_SSD1306</td>
        <td>https://github.com/adafruit/Adafruit_SSD1306</td>
        <td>False</td>
    </tr>
</table><hr>
<a name="teensy_avr_teensy41"></a><b>Core:</b> <a h_ref="N/A">Teensyduino</a><br />
<b>Version:</b> 1.8.5<br />
<b>Board:</b> Teensy 4.1<br />
<br />
<b>Flash:</b> code: 76768 bytes, data: 8200 bytes, headers: 8212 bytes<br />
<b>RAM1:</b> variables: 45760 bytes, data: 73784 bytes, headers: 24520 bytes<br />
<b>RAM2:</b> variables: 13952 bytes<br />
<br />
<b>USB Type:</b> Serial<br />
<b>CPU Speed:</b> 600 MHz<br />
<b>Optimize:</b> Faster<br />
<b>Keyboard Layout:</b> US English<br />
<br />
<b>Used libraries:</b><br />
<table>
    <tr>
        <td>Name</td>
        <td>Version</td>
        <td>Url</td>
        <td>Git</td>
        <td>Core library</td>
    </tr>
    <tr>
        <td>Audio</td>
        <td>1.3</td>
        <td>http://www.pjrc.com/teensy/td_libs_Audio.html</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>SPI</td>
        <td>1.0</td>
        <td>http://www.arduino.cc/en/Reference/SPI</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>SD</td>
        <td>2.0.0</td>
        <td>https://github.com/PaulStoffregen/SD</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>SerialFlash</td>
        <td>0.5</td>
        <td>https://github.com/PaulStoffregen/SerialFlash</td>
        <td></td>
        <td>True</td>
    </tr>
    <tr>
        <td>ML SynthTools</td>
        <td>1.3.1</td>
        <td>https://github.com/marcel-licence/ML_SynthTools</td>
        <td>https://github.com/marcel-licence/ML_SynthTools.git</td>
        <td>False</td>
    </tr>
    <tr>
        <td>Wire</td>
        <td>1.0</td>
        <td>http://www.arduino.cc/en/Reference/Wire</td>
        <td></td>
        <td>True</td>
    </tr>
</table>