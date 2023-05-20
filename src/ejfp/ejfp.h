//
// ejfp.h
//
// Created on: 2023-05-20
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_EJFP_H_
#define EJFP_EJFP_H_

#include <jsmn/jsmn_fwd.h>

/// @brief Instance of EJFP
typedef struct {
	/// @brief Holding an instance of `jsmn_parser` allows for stateful parsing
	jsmn_parser jsmnParser;
} Ejfp;

#ifdef __cplusplus
extern "C" {
#endif  // __cplusplus

void ejfpInitialize(Ejfp *aEjfp);

#ifdef __cplusplus
}
#endif  // __cplusplus

#endif  // EJFP_EJFP_H_
