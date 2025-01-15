# Normally this makefile shouldn't be called directly and we expect the output
# path to be on a certain location to fit together with the other OP-TEE
# gits and helper scripts.
ifeq ($O,)
$(error output path should be specified when calling this makefile)
endif

# Prevent use of LDFLAGS from the environment. For example, yocto exports
# LDFLAGS that are suitable for the client applications, not for TAs
LDFLAGS=

TA_DIRS := testapp \
	   testapp_storage \

.PHONY: all
all: ta

.PHONY: ta
ta: 
	$(q)$(foreach dir,$(TA_DIRS), $(MAKE) -C $(dir) O=$(O)/$(dir) &&) true

.PHONY: clean
clean:
	$(q)$(foreach dir,$(TA_DIRS), $(MAKE) -C $(dir) O=$(O)/$(dir) $@ &&) true
