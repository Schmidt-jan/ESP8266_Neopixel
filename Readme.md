# Neopixel
This project should control a WS2812 LED lightstrip via an ESP8266 which is using FreeRTOS as an operating system.

## Configuration
Make sure you have installed the [ESP8266_RTOS_SDK](https://github.com/espressif/ESP8266_RTOS_SDK) and the compiler.
Therefore you should clone the repository end execute the `install.sh` and `export.sh` scripts.  
Verify that your shell has the environment variable `IDF_PATH` and that your `PATH` contains the compiler directory.

1. Run `make menuconfig`
    - Enter `Serial flasher config` → Change QIO of SPI Mode to DIO (You can try to use the QIO but this caused some partition table error [see here](https://github.com/espressif/ESP8266_RTOS_SDK/issues/929))
    - Enter `Project Configuration` → Enter your settings
    - Enter `Component config` → `Heap memory` → Enable `Disable IRAM as heap memory`



2. Run `make flash -j 16`. To flash the program. (The `-j 16` speeds the compiling up by using 16 threads)
3. Run `make simple_monitor` to see the logs and outputs

### VSCode
If you are using VSCode, the `tasks.json` contains a list with all important commands. Press `CTRL`+ `SHIFT` + `P` → `Tasks: Run Task` to execute one.

