# Enter lower-power sleep mode when on the ChibiOS idle thread
OPT_DEFS += -DCORTEX_ENABLE_WFI_IDLE=TRUE

VPATH += $(TOP_DIR)/keyboards/keychron/common

SRC += common/74hc595_matrix.c
