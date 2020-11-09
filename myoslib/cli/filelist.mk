###########################################################
# Libary Sources
###########################################################

# Set freertos folder path.
MY_PATH= ../../myoslib/cli

# Set folder path with header
APP_CFLAGS += -I$(MY_PATH)

# List of all c files added
APPLICATION_SRCS += $(MY_PATH)/s4431800_cli.c
APPLICATION_SRCS += $(MY_PATH)/s4431800_os.c