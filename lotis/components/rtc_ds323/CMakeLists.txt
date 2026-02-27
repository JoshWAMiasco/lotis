idf_component_register(SRCS "rtc_ds323.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    REQUIRES i2c_helper)

