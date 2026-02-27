file(GLOB_RECURSE srcs "main.c" "src/*.c")

idf_component_register(SRCS "${srcs}"
                       PRIV_REQUIRES bt nvs_flash esp_driver_gpio
                       REQUIRES i2c_helper
                       REQUIRES liquid_crystal_i2c
                       REQUIRES rtc_ds323
                       REQUIRES sd_card
                       REQUIRES sim900
                       REQUIRES json
                       INCLUDE_DIRS "./include")
