//
// serialization.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_SERIALIZATION_H_
#define EJFP_SERIALIZATION_H_

#include "fieldVariant.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

/// @brief
/// @return 0, if failed. Output size otherwise
size_t ejfpSerialize(EjfpFieldVariant *aFieldVariants, const size_t aFieldVariantsSize, char *aOut,
	const size_t aOutSize);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EJFP_SERIALIZATION_H_
