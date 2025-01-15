#ifndef TA_SDP_H
#define TA_SDP_H

/* This UUID is generated with uuidgen
   the ITU-T UUID generator at http://www.itu.int/ITU-T/asn1/uuid.html */
#define TA_SDP_UUID { 0xb9aa5f00, 0xd229, 0x11e4, \
		{ 0x92, 0x5c, 0x00, 0x02, 0xa5, 0xd5, 0xc5, 0x1b} }

/**
 * TA_SDP_CREATE_REGION have 3 parameters:
 * - TEE_PARAM_TYPE_VALUE_INPUT
 *      param[0].value.a: memory region address MSB (to be tested for 64bits)
 *      param[0].value.b: memory region address LSB
 * - TEE_PARAM_TYPE_VALUE_INPUT
 *		param[1].value.a: size of the memory region
 * - TEE_PARAM_TYPE_VALUE_OUTPUT
 *		param[2].value.a: region identifier
 */
#define TA_SDP_CREATE_REGION    0

/**
 * TA_SDP_DESTROY_REGION have 1 parameter:
 * - TEE_PARAM_TYPE_VALUE_INPUT
 *		param[0].value.a: region identifier
 */
#define TA_SDP_DESTROY_REGION   1

/*
 * TA_SDP_UPDATE_REGION have 3 parameters
 * - TEE_PARAM_TYPE_VALUE_INPUT
 *		param[0].value.a: region identifier
 *		param[0].value.b: inditicated if the permissions have to be added or
 *		removed to the memory region
 * - TEE_PARAM_TYPE_MEMREF_INPUT
 *		params[1].memref.buffer: the device name
 *		params[1].memref.size: lenght of the string
 * - TEE_PARAM_TYPE_VALUE_INPUT
 *		params[2].value.a: access request direction (read/write)
 */
#define TA_SDP_UPDATE_REGION    2

/*
 * TA_SDP_DUMP_STATUS have 1 parameter
 * - TEE_PARAM_TYPE_MEMREF_OUTPUT
 *		params[0].memref.buffer: status string
 *		params[0].memref.size: lenght of the string
 */
#define TA_SDP_DUMP_STATUS		3

#endif /*TA_SDP_H*/
