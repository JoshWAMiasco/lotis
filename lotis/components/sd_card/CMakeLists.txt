idf_component_register(SRCS "sd_card.c"
                    INCLUDE_DIRS "include"
                    REQUIRES sd_card
                    REQUIRES driver
                    REQUIRES fatfs)

