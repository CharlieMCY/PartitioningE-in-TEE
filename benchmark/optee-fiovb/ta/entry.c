// SPDX-License-Identifier: BSD-2-Clause
/* Copyright (c) 2018, Linaro Limited */
/* Copyright (c) 2019, Foundries.IO */

// #include <config.h>
#include <ta_fiovb.h>
#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <stdlib.h>
#include <string.h>
#include <util.h>

#define DEFAULT_LOCK_STATE	0
#define MAX_SIMPLE_VALUE_SIZE	64

static const uint32_t storageid = TEE_STORAGE_PRIVATE_RPMB;
static const char *named_value_prefix = "named_value_";

/* Prefix used for vendor variables */
#ifdef CFG_FIOVB_VENDOR_PREFIX
static const char *vendor_prefix = TO_STR(CFG_FIOVB_VENDOR_PREFIX) "_";
#endif

static TEE_Result get_named_object_name(char *name_orig,
					uint32_t name_orig_size,
					char *name, uint32_t *name_size)
{
	size_t pref_len = strlen(named_value_prefix);

	if (name_orig_size + pref_len >
	    TEE_OBJECT_ID_MAX_LEN)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Start with prefix */
	TEE_MemMove(name, named_value_prefix, pref_len);

	/* Concatenate provided object name */
	TEE_MemMove(name + pref_len, name_orig, name_orig_size);

	*name_size = name_orig_size + pref_len;

	return TEE_SUCCESS;
}

static TEE_Result check_valid_value(char *val)
{
	const char *valid_values[] = PERSIST_VALUE_LIST;
	unsigned int i;

#ifdef CFG_FIOVB_VENDOR_PREFIX
	/* Allow vendor variables as CFG_FIOVB_VENDOR_PREFIX is set */
	if (!strncmp(val, vendor_prefix, strlen(vendor_prefix)))
		return TEE_SUCCESS;
#endif

	for (i = 0; i < ARRAY_SIZE(valid_values); i++) {
		if (strcmp(val, valid_values[i]) == 0)
			return TEE_SUCCESS;
	}

	return TEE_ERROR_ITEM_NOT_FOUND;
}

static TEE_Result write_value(char *name, uint32_t name_sz,
			      char *value, uint32_t value_sz,
			      bool overwrite)
{
	TEE_ObjectHandle h = TEE_HANDLE_NULL;
	TEE_Result res = TEE_SUCCESS;
	char name_full[TEE_OBJECT_ID_MAX_LEN] = { };
	uint32_t name_full_sz = 0;
#ifdef CFG_FIOVB_VENDOR_CREATE
	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
			 TEE_DATA_FLAG_ACCESS_WRITE;
#else
	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ;
#endif

	if (overwrite)
		flags |= TEE_DATA_FLAG_ACCESS_WRITE |
			 TEE_DATA_FLAG_OVERWRITE;

	res = get_named_object_name(name, name_sz,
				    name_full, &name_full_sz);
	if (res)
		return res;

	res = TEE_CreatePersistentObject(storageid, name_full,
					 name_full_sz,
					 flags, NULL, value,
					 value_sz, &h);
	if (res == TEE_ERROR_ACCESS_CONFLICT)
		EMSG("Can't update named object '%s' value, res = 0x%x",
		     name, res);
	else if (res)
		EMSG("Can't create named object '%s' value, res = 0x%x",
		     name, res);

	TEE_CloseObject(h);

	return res;
}

static TEE_Result read_value(char *name, uint32_t name_sz,
			     char *value, uint32_t value_sz,
			     uint32_t *count)
{
	TEE_ObjectHandle h = TEE_HANDLE_NULL;
	TEE_Result res = TEE_SUCCESS;
	uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
			 TEE_DATA_FLAG_ACCESS_WRITE;
	char name_full[TEE_OBJECT_ID_MAX_LEN];
	uint32_t name_full_sz = 0;

	res = get_named_object_name(name, name_sz,
				    name_full, &name_full_sz);
	if (res)
		return res;

	res = TEE_OpenPersistentObject(storageid, name_full,
				       name_full_sz, flags, &h);
	if (res) {
		DMSG("Can't open named object '%s' value, res = 0x%x", name, res);
		return res;
	}

	res =  TEE_ReadObjectData(h, value, value_sz, count);
	if (res) {
		EMSG("Can't read named object '%s' value, res = 0x%x", name, res);
	}

	TEE_CloseObject(h);

	return res;
}

static TEE_Result delete_value(char *name, size_t name_sz)
{
	TEE_ObjectHandle h = TEE_HANDLE_NULL;
	TEE_Result res = TEE_SUCCESS;
	const uint32_t flags = TEE_DATA_FLAG_ACCESS_READ |
			       TEE_DATA_FLAG_ACCESS_WRITE_META;
	char name_full[TEE_OBJECT_ID_MAX_LEN] = { };
	uint32_t name_full_sz = 0;

	res = get_named_object_name(name, name_sz,
				    name_full, &name_full_sz);
	if (res)
		return res;

	res = TEE_OpenPersistentObject(storageid, name_full,
				       name_full_sz, flags, &h);
	if (res) {
		EMSG("Failed to open persistent object, res = 0x%x", res);
		return res;
	}

	res = TEE_CloseAndDeletePersistentObject1(h);
	if (res)
		EMSG("Failed to delete persistent object, res = 0x%x", res);

	return res;
}

static bool is_rollback_protected(void)
{
	TEE_Result res = TEE_SUCCESS;
	uint32_t value_sz = MAX_SIMPLE_VALUE_SIZE;
	char value[MAX_SIMPLE_VALUE_SIZE];
	uint32_t count;

	res = read_value(ROLLBACK_PROT, strlen(ROLLBACK_PROT) + 1,
			value, value_sz, &count);
	if (res == TEE_SUCCESS) {
		DMSG("Found %s, rollback protection is enabled",
		      ROLLBACK_PROT);
		return true;
	}

	return false;
}

static bool is_version_incremental(char *new_ver_str,
				   uint32_t new_ver_sz)
{
	TEE_Result res = TEE_SUCCESS;
	char value[MAX_SIMPLE_VALUE_SIZE];
	uint32_t count;
	uint64_t current_ver, new_ver;

	res = read_value(BOOTFIRM_VER, strlen(BOOTFIRM_VER) + 1,
			value, MAX_SIMPLE_VALUE_SIZE, &count);
	if (res == TEE_ERROR_ITEM_NOT_FOUND) {
		DMSG("Not found %s, writing firmware version first time",
		     BOOTFIRM_VER);
		return true;
	}

	if (res == TEE_SUCCESS) {
		current_ver = strtoul(value, NULL, 10);
		new_ver = strtoul(new_ver_str, NULL, 10);

		DMSG("Trying to update boot firmware version, old = %"PRIu64
		     " new = %"PRIu64, current_ver, new_ver);
		if (new_ver >= current_ver)
			return true;
	}

	return false;
}

static TEE_Result increase_boot_firmware(char *new_ver_str,
					 uint32_t new_ver_sz)
{
	if (is_rollback_protected() &&
	    !is_version_incremental(new_ver_str, new_ver_sz)) {
		EMSG("Boot firmware version update is not permitted");
		return TEE_ERROR_ACCESS_DENIED;
	}

	return write_value(BOOTFIRM_VER, strlen(BOOTFIRM_VER) + 1,
			   new_ver_str, new_ver_sz, true);
}

static TEE_Result write_persist_value(uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	TEE_Result res = TEE_SUCCESS;
	uint32_t name_buf_sz = 0;
	uint32_t value_sz = 0;
	char *name_buf = NULL;
	char *value = NULL;
	bool overwrite = true;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	name_buf = params[0].memref.buffer;
	name_buf_sz = params[0].memref.size;

	if (check_valid_value(name_buf) != TEE_SUCCESS) {
		EMSG("Not found %s", name_buf);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	/*
	 * Vendor variables and rollback_protection should not be
	 * allowed to be overwritten
	 */
#ifdef CFG_FIOVB_VENDOR_PREFIX
	if (!strncmp(name_buf, vendor_prefix, strlen(vendor_prefix)))
		overwrite = false;
#endif

	if (!strncmp(name_buf, ROLLBACK_PROT, strlen(ROLLBACK_PROT)))
		overwrite = false;

	value_sz = params[1].memref.size;
	value = TEE_Malloc(value_sz, 0);
	if (!value)
		return TEE_ERROR_OUT_OF_MEMORY;

	TEE_MemMove(value, params[1].memref.buffer,
		    value_sz);

#ifdef CFG_FIOVB_VENDOR_PREFIX
	if (!strncmp(name_buf, vendor_prefix, strlen(vendor_prefix)) &&
	    !IS_ENABLED(CFG_FIOVB_VENDOR_CREATE)) {
		res = TEE_ERROR_BAD_PARAMETERS;

		/* Don't create vendor variables */
		EMSG("Can't create object '%s', CFG_FIOVB_VENDOR_CREATE not set",
		     name_buf);
	} else
#endif
	if (strncmp(name_buf, BOOTFIRM_VER, strlen(BOOTFIRM_VER))) {
		res = write_value(name_buf, name_buf_sz,
				  value, value_sz, overwrite);
	} else {
		/* Handle bootfirmware version change */
		res = increase_boot_firmware(value, value_sz);
	}

	TEE_Free(value);

	return res;
}

static TEE_Result read_persist_value(uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_MEMREF_INOUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	TEE_Result res = TEE_SUCCESS;
	uint32_t name_buf_sz = 0;
	char *name_buf = NULL;
	uint32_t value_sz = 0;
	char *value = NULL;
	uint32_t count = 0;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	name_buf = params[0].memref.buffer;
	name_buf_sz = params[0].memref.size;

	if (check_valid_value(name_buf) != TEE_SUCCESS) {
		EMSG("Not found %s", name_buf);
		return TEE_ERROR_BAD_PARAMETERS;
	}

	value_sz = params[1].memref.size;
	value = TEE_Malloc(value_sz, 0);
	if (!value)
		return TEE_ERROR_OUT_OF_MEMORY;

	res = read_value(name_buf, name_buf_sz, value, value_sz, &count);

	TEE_MemMove(params[1].memref.buffer, value,
		    value_sz);

	params[1].memref.size = count;

	TEE_Free(value);

	return res;
}

static TEE_Result delete_persist_value(uint32_t pt,
				       TEE_Param params[TEE_NUM_PARAMS])
{
	const uint32_t exp_pt = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INPUT,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE,
						TEE_PARAM_TYPE_NONE);
	TEE_Result res;
	char *name_buf;
	size_t name_buf_sz;

	if (pt != exp_pt)
		return TEE_ERROR_BAD_PARAMETERS;

	name_buf = params[0].memref.buffer;
	name_buf_sz = params[0].memref.size;

	/*
	 * rollback_protection should not be allowed to be deleted
	 */
	if (!strncmp(name_buf, ROLLBACK_PROT, strlen(ROLLBACK_PROT)))
		return TEE_ERROR_ACCESS_DENIED;

	res = delete_value(name_buf, name_buf_sz);

	return res;
}

TEE_Result TA_CreateEntryPoint(void)
{
	return TEE_SUCCESS;
}

void TA_DestroyEntryPoint(void)
{
}

TEE_Result TA_OpenSessionEntryPoint(uint32_t pt __unused,
				    TEE_Param params[4] __unused,
				    void **session __unused)
{
	return TEE_SUCCESS;
}

void TA_CloseSessionEntryPoint(void *sess __unused)
{
}

TEE_Result TA_InvokeCommandEntryPoint(void *sess __unused, uint32_t cmd,
				      uint32_t pt,
				      TEE_Param params[TEE_NUM_PARAMS])
{
	switch (cmd) {
	case TA_FIOVB_CMD_READ_PERSIST_VALUE:
		return read_persist_value(pt, params);
	case TA_FIOVB_CMD_WRITE_PERSIST_VALUE:
		return write_persist_value(pt, params);
	case TA_FIOVB_CMD_DELETE_PERSIST_VALUE:
		return delete_persist_value(pt, params);
	default:
		EMSG("Command ID 0x%x is not supported", cmd);
		return TEE_ERROR_NOT_SUPPORTED;
	}
}
