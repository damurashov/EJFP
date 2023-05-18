//
// fieldVariant.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_FIELDVARIANT_H_
#define EJFP_FIELDVARIANT_H_

typedef enum {
	EjfpFieldVariantTypeInteger = 0,
	EjfpFieldVariantTypeBoolean,
	EjfpFieldVariantTypeString,
} EjfpFieldVariantType;

typedef struct {
	EjfpFieldVariantType fieldType;
	const char *fieldName;
	union {
		const char *stringValue;
		int integerValue;
		int booleanValue;
	};
} EjfpFieldVariant;

#endif  // EJFP_FIELDVARIANT_H_
