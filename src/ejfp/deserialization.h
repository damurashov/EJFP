//
// deserialization.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_DESERIALIZATION_H_
#define EJFP_DESERIALIZATION_H_

#include "ejfp/ejfp.h"
#include "ejfp/fieldVariant.h"

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// @brief Deserializes bytes into an array of `EjfpFieldVariant` instances
///
/// @return Number of filled tokens in `EjfpFieldVariant`. Error code otherwise
int ejfpDeserialize(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	const char *aInputBuffer, size_t aInputBufferSize);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EJFP_DESERIALIZATION_H_
