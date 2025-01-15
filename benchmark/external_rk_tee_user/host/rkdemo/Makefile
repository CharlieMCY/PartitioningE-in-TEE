# Normally this makefile shouldn't be called directly and we expect the output
# path to be on a certain location to fit together with the other OP-TEE
# gits and helper scripts.
CC		?= $(CROSS_COMPILE)gcc
CPP		?= $(CROSS_COMPILE)cpp
LD		?= $(CROSS_COMPILE)ld
AR		?= $(CROSS_COMPILE)ar
NM		?= $(CROSS_COMPILE)nm
OBJCOPY		?= $(CROSS_COMPILE)objcopy
OBJDUMP		?= $(CROSS_COMPILE)objdump
READELF		?= $(CROSS_COMPILE)readelf

objs := rkdemo_ca.o

objs1 := rkdemo_storage_ca.o

CFLAGS += -Wall -I../../ta/testapp/include -I../../ta/testapp_storage/include -I$(TEEC_EXPORT)/include -I./include
#Add/link other required libraries here
LDADD += -lteec -L$(TEEC_EXPORT)/lib -lpthread

.PHONY: all
all: $(BINARY)

$(BINARY): $(OBJS)
	$(CC) -o $@ $< $(LDADD)

.PHONY: all
all: rkdemo

rkdemo: testapp testapp_storage

testapp: $(objs)
	$(CC) -o $@ $< $(LDADD)

testapp_storage: $(objs1)
	$(CC) -o $@ $< $(LDADD)

.PHONY: clean
clean:
	rm -f $(OBJS) $(BINARY)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@
