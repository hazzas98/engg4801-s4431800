##############################################################################
# Application Settings
##############################################################################

PROJ_NAME     		:= thesis
SUPPORTED_TARGETS 	:= thunderboard2 nrf52840dk nrf52832dk bleatag argon xenon

##############################################################################
# Application Specific Flags
##############################################################################
	
APP_CFLAGS			:= -Wno-pointer-sign

##############################################################################
# Application Specific Library Source Files
##############################################################################

CORE_CSIRO          := $(realpath ./../../core_csiro)
APPLICATION_SRCS 	:= 
override REPO_ROOT = ../../../ei-freertos

##############################################################################
# Main Build Rules
##############################################################################

include ../../ei_changes/freertos_cli/filelist.mk
include ../../myoslib/spi/filelist.mk
include ../../myoslib/cli/filelist.mk
include ../../myoslib/state/filelist.mk
include ../../myoslib/uart/filelist.mk
include ../../myoslib/adc/filelist.mk
include ../../../ei-freertos/core_csiro/m_common.mk