//
// fieldVariant.h
//
// Created on: 2023-05-18
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_FIELDVARIANT_H_
#define EJFP_FIELDVARIANT_H_

typedef enum EjfpFieldVariantType {
	EjfpFieldVariantTypeInteger = 0,
	EjfpFieldVariantTypeBoolean,
	EjfpFieldVariantTypeString,
};

typedef struct EjfpFieldVariant {
	union {
		const char *stringValue;
		int integerValue;
		int booleanValue;
	};
	const char *fieldName;
	EjfpFieldVariantType fieldType;
};

#endif  // EJFP_FIELDVARIANT_H_
