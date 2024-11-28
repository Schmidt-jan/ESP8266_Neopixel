# Neopixel
This project should control a WS2812 LED lightstrip via an ESP8266 which is using FreeRTOS as an operating system.

## Configuration
Make sure you have installed the [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK) and the compiler.
Therefore you should clone the repository end execute the `install.sh` and `export.sh` scripts.  
Verify that your shell has the environment variable `IDF_PATH` and that your `PATH` contains the compiler directory.

1. Run `make menuconfig`
    - Enter `Serial flasher config` → Change QIO of SPI Mode to DIO (You can try to use the QIO but this caused some partition table error [see here](https://github.com/espressif/ESP8266_RTOS_SDK/issues/929))
    - Enter `Project Configuration` 
        → Enter your settings
    - Enter `Component config`
        - → `Heap memory` → Enable `Disable IRAM as heap memory`
        - → `SPIFFS configuration` 
            - Disable `Enable SPIFFS Filesystem Magic`
            - `Size of per-file metadata field` = 0



2. Run `make flash -j 16`. To flash the program. (The `-j 16` speeds the compiling up by using 16 threads)
3. Run `make simple_monitor` to see the logs and outputs

### VSCode
If you are using VSCode, the `tasks.json` contains a list with all important commands. Press `CTRL`+ `SHIFT` + `P` → `Tasks: Run Task` to execute one.

### [mkspiffs](https://github.com/igrr/mkspiffs)
To save static data you can use the spiffs interface. Clone the [mkspiffs](https://github.com/igrr/mkspiffs) repository
1. Build a executable for the ESP8266 with the following configurations:
- SPIFFS_OBJ_META_LEN=0 
- SPIFFS_USE_MAGIC=0 
- SPIFFS_USE_MAGIC_LENGTH=0 
- SPIFFS_ALIGNED_OBJECT_INDEX_TABLES=1  
**Example:**  
`make dist CPPFLAGS="-DSPIFFS_OBJ_META_LEN=0 -DSPIFFS_USE_MAGIC=0 -DSPIFFS_USE_MAGIC_LENGTH=0 -DSPIFFS_ALIGNED_OBJECT_INDEX_TABLES=1" BUILD_CONFIG_NAME=-custom`


2. Generate the binary:  
`mkspiffs-0.2.3-7-gf248296-custom-linux64/mkspiffs -c ../../data/ -b 4096 -p 256 -s 0x64000 -d 5 ../../build/spiffs.bin`  
3. Flash the binary:  
`python3 $IDF_PATH/components/esptool_py/esptool/esptool.py --chip esp8266 --port /dev/ttyUSB0 --baud 115200 write_flash 0x8d000 ../../build/spiffs.bin`

If you change the [partitions.csv](partitions.csv) mind changing the size of the `spiffs.bin` (0x64d000) and updating the spiffs destination (0x8d000).