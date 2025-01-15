/*
 * sdp_ta.c
 *
 */

#define STR_TRACE_USER_TA "SDP"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include "ta_sdp.h"
#include "sdp_platform_api.h"
#include "string_ext.h"

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	platform_init();
	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param  params[4], void **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye SDP\n");
}

static TEE_Result create_region(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_VALUE_OUTPUT,
							TEE_PARAM_TYPE_NONE);
	uint64_t addr;
	int index;

	if (param_types != exp_param_types) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	addr = params[0].value.b;

	index = platform_create_region(addr, params[1].value.a);
	if (index < 0)
		return TEE_ERROR_BAD_PARAMETERS;

	params[2].value.a = index;

	return TEE_SUCCESS;
}

static TEE_Result destroy_region(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE,
							TEE_PARAM_TYPE_NONE);
	uint32_t id;

	if (param_types != exp_param_types) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	id = params[0].value.a;
	platform_destroy_region(id);

	return TEE_SUCCESS;
}

static TEE_Result update_region(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_MEMREF_INPUT,
							TEE_PARAM_TYPE_VALUE_INPUT,
							TEE_PARAM_TYPE_NONE);
	uint32_t region_id;
	bool add;
	int dir;
	char *name;
	struct secure_device *device;
	struct region *region;

	if (param_types != exp_param_types) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	region_id = params[0].value.a;
	add = params[0].value.b;

	name = params[1].memref.buffer;

	dir = params[2].value.a;

	device = platform_find_device_by_name(name);
	if (device == 0) {
		IMSG("Can't find device %s\n", name);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	region = platform_find_region_by_id(region_id);
	if (region == NULL) {
		IMSG("Can't find region id %d\n", region_id);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	if (add) {
		if (platform_check_permissions(region, device, dir)) {
			IMSG("check permissions failed\n");
			return TEE_ERROR_BAD_PARAMETERS;
		}

		platform_add_device_to_region(region, device, dir);
	} else {
		platform_remove_device_from_region(region, device);
	}

	return TEE_SUCCESS;
}

static TEE_Result dump_status(uint32_t param_types, TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_OUTPUT,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE,
											   TEE_PARAM_TYPE_NONE);
	if (param_types != exp_param_types) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	platform_dump_status(params, params[0].memref.size);
	platform_dump_device(params[0].memref.buffer, params[0].memref.size);

	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void *sess_ctx, uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */


	switch (cmd_id) {
	case TA_SDP_CREATE_REGION:
		return create_region(param_types, params);
	case TA_SDP_DESTROY_REGION:
		return destroy_region(param_types, params);
	case TA_SDP_UPDATE_REGION:
		return update_region(param_types, params);
	case TA_SDP_DUMP_STATUS:
		return dump_status(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
