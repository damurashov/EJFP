//
// error.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_ERROR_H_
#define EJFP_ERROR_H_

typedef enum {
	EjfpOk = 0,
	EjfpErrorSerializationNoMemory = -1,
	EjfpErrorDeserializationInvalidSyntax = -2,
	EjfpErrorDeserializationPartitioned = -3,
	EjfpErrorDeserializationNoMemory = -4,
	EjfpErrorDeserializationUnsupportedJsonStructure = -5,  // EJFP does not support complicated JSON structures
} EjfpError;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// @brief  Get last error code
EjfpError ejfpErrorCode();

/// @brief Set error code
void ejfpSetErrorCode(EjfpError aEjfpError);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EJFP_ERROR_H_
