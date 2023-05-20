//
// fieldVariant.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_FIELDVARIANT_H_
#define EJFP_FIELDVARIANT_H_

#include <stddef.h>

typedef enum {
	EjfpFieldVariantTypeInteger = 0,
	EjfpFieldVariantTypeBoolean,
	EjfpFieldVariantTypeString,
	EjfpFieldVariantTypeFloat,
	EjfpFieldVariantTypeNull,
} EjfpFieldVariantType;

typedef struct {
	EjfpFieldVariantType fieldType;
	const char *fieldName;
	union {
		int integerValue;
		int booleanValue;
		const char *stringValue;
		float floatValue;
	};

	/// @brief Required for deserialization, when the string is not null-terminated
	size_t fieldNameLength;

	/// @brief Required for deserialization, when the string is not null-terminated
	size_t stringValueLength;
} EjfpFieldVariant;

#endif  // EJFP_FIELDVARIANT_H_
