idf_component_register(SRCS "liquid_crystal_i2c.c"
					REQUIRES i2c_helper
					REQUIRES driver
                    INCLUDE_DIRS "include")

