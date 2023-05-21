//
// print.h
//
// Created on: 2023-05-21
//     Author: Dmitry Murashov (dmtr <DOT> murashov <AT> <GMAIL>)
//

#ifndef EJFP_PRINT_H_
#define EJFP_PRINT_H_

#include "ejfp/fieldVariant.h"
#include <stdio.h>
#include <string.h>

static inline void ejfpFieldVariantPrint(EjfpFieldVariant const *aEjfpFieldVariant)
{
	// Print field name
	printf("EjfpFieldVariant{\"");

	for (size_t i = 0; i < aEjfpFieldVariant->fieldNameLength; ++i) {
		printf("%c", aEjfpFieldVariant->fieldName[i]);
	}

	printf("\":");

	// Print field value
	switch (aEjfpFieldVariant->fieldType) {
		case EjfpFieldVariantTypeString:
			printf("\"");

			for (size_t i = 0; i < aEjfpFieldVariant->stringValueLength; ++i) {
				printf("%c", aEjfpFieldVariant->stringValue[i]);
			}

			printf("\"");

			break;

		case EjfpFieldVariantTypeFloat:
			printf("%.4f", aEjfpFieldVariant->floatValue);

			break;

		case EjfpFieldVariantTypeNull:
			printf("null");

			break;

		case EjfpFieldVariantTypeInteger:
			printf("%d", aEjfpFieldVariant->integerValue);

			break;

		case EjfpFieldVariantTypeBoolean:
			if (aEjfpFieldVariant->booleanValue) {
				printf("true");
			} else {
				printf("false");
			}

			break;
	}

	printf("}");
}

#ifdef __cplusplus

#include <ostream>

static inline std::ostream &operator<<(std::ostream &aOut, const EjfpFieldVariant &aEjfpFieldVariant)
{
	// Print field name
	aOut << "EjfpFieldVariant{\"";

	for (size_t i = 0; i < aEjfpFieldVariant.fieldNameLength; ++i) {
		aOut << aEjfpFieldVariant.fieldName[i];
	}

	printf("\":");

	// Print field value
	switch (aEjfpFieldVariant.fieldType) {
		case EjfpFieldVariantTypeString:
			printf("\"");

			for (size_t i = 0; i < aEjfpFieldVariant.stringValueLength; ++i) {
				aOut << aEjfpFieldVariant.stringValue[i];
			}

			aOut << "\"";

			break;

		case EjfpFieldVariantTypeFloat:
			aOut << aEjfpFieldVariant.floatValue;

			break;

		case EjfpFieldVariantTypeNull:
			aOut << "null";

			break;

		case EjfpFieldVariantTypeInteger:
			aOut << aEjfpFieldVariant.integerValue;

			break;

		case EjfpFieldVariantTypeBoolean:
			if (aEjfpFieldVariant.booleanValue) {
				aOut << "true";
			} else {
				aOut << "false";
			}

			break;
	}

	aOut << "}";

	return aOut;
}

#endif  // __cplusplus

#endif  // EJFP_PRINT_H_
