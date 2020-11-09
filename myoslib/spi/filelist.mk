###########################################################
# Libary Sources
###########################################################

# Set freertos folder path.
MY_PATH = ../../myoslib/spi

# Set folder path with header
APP_CFLAGS += -I$(MY_PATH)

# List of all c files added
APPLICATION_SRCS += $(MY_PATH)/s4431800_spi.c