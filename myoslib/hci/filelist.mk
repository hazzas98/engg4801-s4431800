###########################################################
# Libary Sources
###########################################################

# Set freertos folder path.
MY_PATH = ../../myoslib/hci

# Set folder path with header
APP_CFLAGS += -I$(MY_PATH)

# List of all c files added
APPLICATION_SRCS += $(MY_PATH)/s4431800_hal_hci.c
APPLICATION_SRCS += $(MY_PATH)/s4431800_os_hci.c