/*
 * sdp_platform_api.h
 *
 */
#ifndef _SDP_PLATFORM_API_H_
#define _SDP_PLATFORM_API_H_

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

/* those definitions need to be aligned
 * with enum dma_data_direction definitions */
#define DIR_RW    0
#define DIR_READ  1
#define DIR_WRITE 2

#define MAX_NAME_SIZE 64

/* define some typical type of device */
#define DECRYPTER   (1 << 24)
#define PARSER      (2 << 24)
#define DECODER     (3 << 24)
#define TRANSFORMER (4 << 24)
#define SINK        (5 << 24)

/* streams type */
#define VIDEO		(1 << 16)
#define AUDIO       (2 << 16)

/* helpers */
#define IS_DECRYPTER(x)   ((x & 0xFF000000) == DECRYPTER)
#define IS_PARSER(x)      ((x & 0xFF000000) == PARSER)
#define IS_DECODER(x)     ((x & 0xFF000000) == DECODER)
#define IS_TRANSFORMER(x) ((x & 0xFF000000) == TRANSFORMER)
#define IS_SINK(x)        ((x & 0xFF000000) == SINK)

#define IS_VIDEO(x)	      ((x & 0x00FF0000) == VIDEO)
#define IS_AUDIO(x)       ((x & 0x00FF0000) == AUDIO)
#define STREAM_TYPE(x)    (x & 0x00FF0000)

/**
 * struct secure_device - opaque structure where each
 * platform can implement it own services
 */
struct secure_device;

/**
 * struct region - opaque structure to be customize
 * by each platform to macth with hardware requirement
 */
struct region;

/* prototypes of platform specific functions which need
 * to be implemented by the platform */

/**
 * platform_init - call when the instance of TA is created
 */
int platform_init(void);

/**
 * platform_create_region - request the creation of a region
 *
 * @addr: start address of the memory
 * @size: lenght of the memory
 *
 * if success return a (unique) region identifier
 * else return a negative value
 */
int platform_create_region(uint64_t addr, uint32_t size);

/**
 * platform_destroy_region - destroy a specific region
 *
 * @id: the region identifier to be destroy
 *
 * return 0 if success
 */
int platform_destroy_region(int id);

/**
 * platform_find_region_by_id - find a region by using it identifier
 * identifier should have been provide by platform_create_region()
 *
 * @id: the region identifier to be find
 *
 * return a struct region * if the region has been found
 * else return NULL
 */
struct region *platform_find_region_by_id(int id);

/**
 * platform_find_device_by_name - find a device by using it name
 *
 * @name: the name of the device (device->driver->name)
 *
 * return a struct secure_device * if the device has been found
 * else return NULL
 */
struct secure_device *platform_find_device_by_name(char *name);

/**
 * platform_check_permissions - check if the given device can have access
 * to a specific region
 *
 * @region: targeted region
 * @device: the device requesting the access
 * @dir: access direction (read and/or write)
 */
int platform_check_permissions(struct region *region, struct secure_device* device, int dir);

/**
 * platform_add_device_to_region - add device access to a region
 * call after platform_check_permissions() to set access to specific region
 * for the device
 *
 * @region: targeted region
 * @device: the device requesting the access
 * @dir: access direction (read and/or write)
 */
int platform_add_device_to_region(struct region *region, struct secure_device* device, int dir);

/**
 * platform_remove_device_from_region - remove a device from a region
 *
 * @region: targeted region
 * @device: the device to be remove
 */
int platform_remove_device_from_region(struct region *region, struct secure_device* device);

/**
 * platform_dump_status - request to platform code to write a status
 *
 * @dump: string to be filled by platform code
 * @size: size of the buffer
 */
int platform_dump_status(TEE_Param params[4], int size);

int platform_dump_device(char *dump, int size);

#endif
