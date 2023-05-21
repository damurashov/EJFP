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
	EjfpFieldVariantTypeUninitialized = 0,
	EjfpFieldVariantTypeInteger,
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

	/// @brief Required for deserialization, when the string is not
	/// null-terminated. Must always be initialized.
	///
	/// @pre If 0, `fieldName` is a NULL-terminated string. Otherwise, this
	/// value MUST be equal to the actual string length.
	size_t fieldNameLength;

	/// @brief Required for deserialization, when the string is not
	/// null-terminated.
	///
	/// @pre If 0, `stringValue` is a NULL-terminated string. Otherwise, this
	/// value MUST be equal to the actual string length
	size_t stringValueLength;
} EjfpFieldVariant;

#endif  // EJFP_FIELDVARIANT_H_
