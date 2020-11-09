###########################################################
# Libary Sources
###########################################################

# Set freertos folder path.
MY_PATH= ../../ei_changes/freertos_cli

# Set folder path with header
APP_CFLAGS += -I$(MY_PATH)

# List of all c files added
APPLICATION_SRCS += $(MY_PATH)/FreeRTOS_CLI.c