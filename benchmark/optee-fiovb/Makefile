export V ?= 0

TEE_SDK_DIR=$(shell pwd)
include ${TEE_SDK_DIR}/../../config.mk

HOST_CROSS_COMPILE ?= $(CROSS_COMPILE)
TA_CROSS_COMPILE ?= $(CROSS_COMPILE)

# ifdef CFG_FIOVB_VENDOR_CREATE
# ifndef CFG_FIOVB_VENDOR_PREFIX
# $(error CFG_FIOVB_VENDOR_CREATE is set but no CFG_FIOVB_VENDOR_PREFIX was provided)
# endif
# endif

.PHONY: all
all:
	$(MAKE) -C host CROSS_COMPILE="$(HOST_CROSS_COMPILE)" --no-builtin-variables  \
		TEEC_EXPORT=${TEE_SDK_DIR}/../../optee_client/out/export
	$(MAKE) -C ta CROSS_COMPILE="$(TA_CROSS_COMPILE)" LDFLAGS=""  \
		TA_DEV_KIT_DIR=${TEE_SDK_DIR}/../../optee_os/out/arm/export-ta_arm32

.PHONY: clean
clean:
	$(MAKE) -C host clean
	$(MAKE) -C ta clean \
		TA_DEV_KIT_DIR=${TEE_SDK_DIR}/../../optee_os/out/arm/export-ta_arm32
