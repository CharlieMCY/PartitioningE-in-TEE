/*
 * smaf-optee.c
 *
 * Copyright (C) Linaro SA 2015
 * Author: Benjamin Gaignard <benjamin.gaignard@linaro.org> for Linaro.
 * License terms:  GNU General Public License (GPL), version 2
 */
/* TODO: cleanup include directories */
#include <tee_client_api.h>

/* Those define are copied from ta_sdp.h */
#define TA_SDP_UUID { 0xb9aa5f00, 0xd229, 0x11e4, \
		{ 0x92, 0x5c, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

#define TA_SDP_CREATE_REGION    0
#define TA_SDP_DESTROY_REGION   1
#define TA_SDP_UPDATE_REGION    2
#define TA_SDP_DUMP_STATUS	3

/* trusted application call */

/**
 * sdp_ta_create_region -create a region with a given address and size
 *
 * in case of success return a region id (>=0) else -EINVAL
 */
static int sdp_ta_region_create(TEEC_Session *sess, size_t size)
{
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_VALUE_INPUT,
					 TEEC_VALUE_OUTPUT, TEEC_NONE);

	op.params[1].value.a = size;

	res = TEEC_InvokeCommand(sess, TA_SDP_CREATE_REGION,
				 &op, &err_origin);

	return op.params[2].value.a;
}

static int sdp_ta_region_destroy(TEEC_Session *sess)
{
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT, TEEC_NONE,
					 TEEC_NONE, TEEC_NONE);

	op.params[0].value.a = 1;

	res = TEEC_InvokeCommand(sess, TA_SDP_DESTROY_REGION,
				 &op, &err_origin);

	return 0;
}

static int sdp_ta_region_update(TEEC_Session *sess)
{
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_VALUE_INPUT,
					 TEEC_MEMREF_TEMP_INPUT,
					 TEEC_VALUE_INPUT,
					 TEEC_NONE);

	op.params[0].value.a = 1;
	op.params[0].value.b = 2;

	char buf[1000] = {0};

	op.params[1].tmpref.buffer = buf;
	op.params[1].tmpref.size = 100 + 1;

	op.params[2].value.a = 2;

	res = TEEC_InvokeCommand(sess, TA_SDP_UPDATE_REGION,
				 &op, &err_origin);

	return 0;
}

static int sdp_init_session(TEEC_Session *sess, TEEC_Context *ctx)
{
	TEEC_Result res;
	uint32_t err_origin;
	TEEC_UUID uuid = TA_SDP_UUID;

	res = TEEC_InitializeContext(NULL, ctx);

	res = TEEC_OpenSession(ctx, sess, &uuid,
			TEEC_LOGIN_PUBLIC, NULL, NULL, &err_origin);

	return 0;
}

static void sdp_destroy_session(TEEC_Session *sess, TEEC_Context *ctx)
{

	TEEC_CloseSession(sess);
	TEEC_FinalizeContext(ctx);
}

#define MAX_DUMP_SIZE 2048
static int smaf_optee_ta_dump_status(TEEC_Session *sess)
{
	TEEC_Operation op;
	TEEC_Result res;
	uint32_t err_origin;
	char *dump;

	memset(&op, 0, sizeof(op));

	op.paramTypes = TEEC_PARAM_TYPES(TEEC_MEMREF_TEMP_OUTPUT,
					 TEEC_NONE,
					 TEEC_NONE,
					 TEEC_NONE);

	dump = malloc(MAX_DUMP_SIZE);
	op.params[0].tmpref.buffer = (void *)dump;
	op.params[0].tmpref.size = MAX_DUMP_SIZE - 1;

	res = TEEC_InvokeCommand(sess, TA_SDP_DUMP_STATUS,
				 &op, &err_origin);

	printf("%s", dump);

	free(dump);
	return 0;
}

int main()
{
	TEEC_Session sess;
	TEEC_Context ctx;

	sdp_init_session(&sess, &ctx);
	sdp_ta_region_create(&sess, 100);
	sdp_ta_region_update(&sess);
	smaf_optee_ta_dump_status(&sess);
	sdp_ta_region_destroy(&sess);
	sdp_destroy_session(&sess, &ctx);
}
