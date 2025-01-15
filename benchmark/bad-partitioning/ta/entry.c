/*
 * Copyright (c) 2016, Linaro Limited
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <tee_internal_api.h>
#include <tee_internal_api_extensions.h>

#include <hello_world_ta.h>

#include <string.h>

/*
 * Called when the instance of the TA is created. This is the first call in
 * the TA.
 */
TEE_Result TA_CreateEntryPoint(void)
{
	DMSG("has been called");

	return TEE_SUCCESS;
}

/*
 * Called when the instance of the TA is destroyed if the TA has not
 * crashed or panicked. This is the last call in the TA.
 */
void TA_DestroyEntryPoint(void)
{
	DMSG("has been called");
}

/*
 * Called when a new session is opened to the TA. *sess_ctx can be updated
 * with a value to be able to identify this session in subsequent calls to the
 * TA. In this function you will normally do the global initialization for the
 * TA.
 */
TEE_Result TA_OpenSessionEntryPoint(uint32_t param_types,
		TEE_Param __maybe_unused params[4],
		void __maybe_unused **sess_ctx)
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	/* Unused parameters */
	(void)&params;
	(void)&sess_ctx;

	/*
	 * The DMSG() macro is non-standard, TEE Internal API doesn't
	 * specify any means to logging from a TA.
	 */
	IMSG("Hello World!\n");

	/* If return value != TEE_SUCCESS the session will not be created. */
	return TEE_SUCCESS;
}

/*
 * Called when a session is closed, sess_ctx hold the value that was
 * assigned by TA_OpenSessionEntryPoint().
 */
void TA_CloseSessionEntryPoint(void __maybe_unused *sess_ctx)
{
	(void)&sess_ctx; /* Unused parameter */
	IMSG("Goodbye!\n");
}

void enc(char *str)
{

}

void dec(char *str)
{

}

void produce_2(TEE_Param params[4])
{
	// bad partitioning 1: unencrypted data output - directly memmove
	char key2[1000] = "123456";
	char vi[1000] = "abcd";
	int v = 100;

	TEE_MemMove(params[1].memref.buffer, key2, strlen(key2)); //p2-1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key2); //p2-2
	params[0].value.a = v; //p2-3
	params[0].value.a = 10; //p2-4

	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key2, vi); //c9 10
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s-%d", key2, vi, v); //c11 12 13

	enc(key2);
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key2); //c14 -1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key2, vi); //c15 16 -1
}

void produce(TEE_Param params[4])
{
	// bad partitioning 1: unencrypted data output - directly memmove
	char key1[1000] = "123456";
	char vi[1000] = "abcd";
	int v = 100;

	TEE_MemMove(params[1].memref.buffer, key1, strlen(key1)); //p1-1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key1); //p1-2
	params[0].value.a = v; //p1-3
	params[0].value.a = 10; //p1-4

	produce_2(params);

	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key1, vi); //c12
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s-%d", key1, vi, v); //c345

	enc(key1);
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key1); //c6 -1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key1, vi); //c78 -1
}

void produce_3(char *buf, int size)
{
	char key[1000] = "123456";
	TEE_MemMove(buf, key, strlen(key)); //p1-5
	snprintf(buf, size, "%s", key); //p1-6
}

static TEE_Result output(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	char key[1000] = "123456";
	char vi[1000] = "abcd";
	int v = 100;
	
	TEE_MemMove(params[1].memref.buffer, key, strlen(key)); //b1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key); //b2
	params[0].value.a = v; //b3
	params[0].value.a = 10; //b4
	params[0].value.a = params[0].value.b; //b5 -1

	produce(params);
	produce_3(params[1].memref.buffer, params[1].memref.size);

	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key, vi); //f12
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s-%d", key, vi, v); //f345
	params[0].value.a = 10 + v; //f6

	enc(key);
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s", key); //pa1 -1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s", key, vi); //pa23 -1
	snprintf(params[2].memref.buffer, params[2].memref.size, "%s-%s-%d", key, vi, v); //pa456 -1

	return TEE_SUCCESS;
}

void produce_i2(int a, int b, char *buf2, int size2, char *buf3, int size3, TEE_Param params[4])
{
	char *str = TEE_Malloc(1000, 0);
	int tmp_arr[20];

	int *arr_a = TEE_Malloc(a, 0); //p2-1
	int *arr_ref = TEE_Malloc(b, 0); //p2-2
	tmp_arr[a] = 43; //p2-3
	for (int i = 0; i < size2; i++) { //p2-4
		str[i] = ((char *)buf2)[i];
	}

	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size); //p2-5

	char c = str[params[0].value.a - 3]; //c7

	TEE_MemMove(str, params[2].memref.buffer, params[2].memref.size); //c8
}

void produce_i0(int a, int b, char *buf2, int size2, char *buf3, int size3, TEE_Param params[4])
{
	char *str = TEE_Malloc(1000, 0);
	int tmp_arr[20];

	int *arr_a = TEE_Malloc(a, 0); //p1-1
	int *arr_ref = TEE_Malloc(b, 0); //p1-2
	tmp_arr[a] = 43; //p1-3
	for (int i = 0; i < size2; i++) { //p1-4
		str[i] = ((char *)buf2)[i];
	}

	TEE_MemMove(str, buf3, size3); //p1-5

	produce_i2(a, b, buf2, size2, buf3, size3, params);

	char c = str[a - 3]; //c1

	if (size3 > 1000)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_MemMove(str, buf2, size2); //c2
	TEE_MemMove(str, buf3, size3); //c3 -1

	if (size2 < 1000)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_MemMove(buf2, str, 1000); //c4 -1
}

void produce_i3(int i)
{
	char *str = TEE_Malloc(1000, 0);
	int tmp_arr[20];

	int *arr_a = TEE_Malloc(i, 0); //p1-6

	tmp_arr[i] = 43; //p1-7

	char c = str[i - 3]; //c5

	if (i > 1000)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}
	c = str[i - 3]; //c6 -1
}

static TEE_Result input(uint32_t param_types,
	TEE_Param params[4])
{
	uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_VALUE_INPUT,
						   TEE_PARAM_TYPE_MEMREF_INPUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_MEMREF_INOUT);

	DMSG("has been called");

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;

	char *str = TEE_Malloc(1000, 0);
	int tmp_arr[20];

	int *arr_a = TEE_Malloc(params[0].value.a, 0); //b1
	int *arr_ref = TEE_Malloc(params[1].memref.size, 0); //b2
	tmp_arr[params[0].value.a] = 43; //b3
	for (int i = 0; i < params[2].memref.size; i++) { //b4
		str[i] = ((char *)params[2].memref.buffer)[i];
	}

	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size); //b5
	TEE_MemMove(params[2].memref.buffer, str, 1000); //b6

	char c = ((char *)params[3].memref.buffer)[params[3].memref.size - 3]; //b7 -1

	c = str[params[0].value.a - 3]; //f1
	c = str[30 - params[0].value.a]; //f2

	if (params[1].memref.size > 10000)
	{
		return TEE_ERROR_BAD_PARAMETERS;
	}

	int *arr_path = TEE_Malloc(params[1].memref.size, 0); //pa1 -1
	// int *arr_path_1 = TEE_Malloc(params[2].memref.size, 0); //pa2

	TEE_MemMove(str, params[2].memref.buffer, params[2].memref.size); //pa3
	TEE_MemMove(str, params[3].memref.buffer, params[3].memref.size); //pa4

	TEE_MemMove(params[2].memref.buffer, str, 1000); //pa5

	produce_i0(params[0].value.a, params[1].memref.size, params[2].memref.buffer, params[2].memref.size, params[3].memref.buffer, params[3].memref.size, params);
	produce_i3(params[0].value.a);

	return TEE_SUCCESS;
}

void produce_s2(char *buf, int size)
{
	if (strcmp("123456", buf) == 0) //p2-1
	{
		IMSG("Match!\n");
	}

	if (!TEE_MemCompare(buf,
				    "123456",
				    size)) { //p2-2
			IMSG("Pass!\n");
	}
	dec(buf); //p2-3
}

void produce_s(char *buf, int size)
{
	if (strcmp("123456", buf) == 0) //p1-1
	{
		IMSG("Match!\n");
	}
	dec(buf); //p1-2

	if (!TEE_MemCompare(buf,
				    "123456",
				    size)) { //p1-7
			IMSG("Pass!\n");
	}

	if (size > 1000) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	char str[1000] = {0};
	TEE_MemMove(str, buf, size); //c1 -1

	produce_s2(buf, size);
}

void produce_s3(TEE_Param params[4])
{
	void *buf = params[0].memref.buffer; //p1-3
	uint32_t sz = params[0].memref.size; //p1-4 -1
	if (strcmp("123456", buf) == 0) //p1-5
	{
		IMSG("Match!\n");
	}

	dec(buf); //p1-6

	if (!TEE_MemCompare(params[0].memref.buffer,
				    "123456",
				    params[0].memref.size)) { //c4
			IMSG("Pass!\n");
	}

	if (!TEE_MemCompare("123456",
					params[0].memref.buffer,
				    params[0].memref.size)) { //c5
			IMSG("Pass!\n");
	}
	
	if (!TEE_MemCompare("123456",
					buf,
				    sz)) { //c6
			IMSG("Pass!\n");
	}

	if (!strcmp(params[0].memref.buffer,
				    "123456")) { //c7
			IMSG("Pass!\n");
	}

	if (!strcmp("123456", params[0].memref.buffer)) { //c8
			IMSG("Pass!\n");
	}

	if (!strcmp(buf, "123456")) { //c9
			IMSG("Pass!\n");
	}

	if (sz > 1000) {
		return TEE_ERROR_BAD_PARAMETERS;
	}
	char str[1000] = {0};
	TEE_MemMove(str, params[0].memref.buffer, params[0].memref.size); //c2 -1
	
	if (strcmp("123456", str)) //c3 -1
	{
		IMSG("Match!\n");
	}

	return TEE_SUCCESS;
}

static TEE_Result shared_memory(uint32_t param_types,
	TEE_Param params[4])
{
		uint32_t exp_param_types = TEE_PARAM_TYPES(TEE_PARAM_TYPE_MEMREF_INOUT,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE,
						   TEE_PARAM_TYPE_NONE);

	if (param_types != exp_param_types)
		return TEE_ERROR_BAD_PARAMETERS;
	
	void *buf = params[0].memref.buffer; //b1
	uint32_t sz = params[0].memref.size; //b2 -1

	if (strcmp("123456", buf) == 0) //b3
	{
		IMSG("Match!\n");
	}

	TEE_Wait(5000);

	if (strcmp("123456", buf)) //b4
	{
		IMSG("Match!\n");
	}

	if (!TEE_MemCompare(buf,
				    "123456",
				    sz)) { //b5
			IMSG("Pass!\n");
	}

	dec(buf); //b6

	if (!TEE_MemCompare(params[0].memref.buffer,
				    "123456",
				    params[0].memref.size)) { //f1
			IMSG("Pass!\n");
	}

	if (!TEE_MemCompare("123456",
					params[0].memref.buffer,
				    params[0].memref.size)) { //f2
			IMSG("Pass!\n");
	}
	
	if (!TEE_MemCompare("123456",
					buf,
				    sz)) { //f3
			IMSG("Pass!\n");
	}

	if (!strcmp(params[0].memref.buffer,
				    "123456")) { //f4
			IMSG("Pass!\n");
	}

	if (!strcmp("123456", params[0].memref.buffer)) { //f5
			IMSG("Pass!\n");
	}

	if (!strcmp(buf, "123456")) { //f6
			IMSG("Pass!\n");
	}

	produce_s(buf, sz);
	produce_s3(params);

	char str[1000] = {0};

	if (sz > 1000) {
		return TEE_ERROR_BAD_PARAMETERS;
	}

	TEE_MemMove(str, params[0].memref.buffer, params[0].memref.size); //pa1 -1
	
	if (strcmp("123456", str)) //pa2 -1
	{
		IMSG("Match!\n");
	}

	return TEE_SUCCESS;
}

/*
 * Called when a TA is invoked. sess_ctx hold that value that was
 * assigned by TA_OpenSessionEntryPoint(). The rest of the paramters
 * comes from normal world.
 */
TEE_Result TA_InvokeCommandEntryPoint(void __maybe_unused *sess_ctx,
			uint32_t cmd_id,
			uint32_t param_types, TEE_Param params[4])
{
	(void)&sess_ctx; /* Unused parameter */

	switch (cmd_id) {
	case TA_HELLO_WORLD_CMD_OUTPUT:
		return output(param_types, params);
	case TA_HELLO_WORLD_CMD_INPUT:
		return input(param_types, params);
	case TA_HELLO_WORLD_CMD_SHM:
		return shared_memory(param_types, params);
	default:
		return TEE_ERROR_BAD_PARAMETERS;
	}
}
