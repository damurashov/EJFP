//
// serialization.c
//
// Created: 2023-05-18
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#include "fieldVariant.h"
#include <mtojson/mtojson.h>

static void tojsonSetObjectMarkerStart(to_json *aInstance);
static void tojsonSetBoolean(to_json *aInstance, const char *aFieldName, int *aValue);
static void tojsonSetInteger(to_json *aInstance, const char *aFieldName, int *aValue);
static void tojsonSetString(to_json *aInstance, const char *aFieldName, const char *aValue);

static inline void tojsonSetObjectMarkerStart(to_json *aInstance)
{
	aInstance->stype = t_to_object;
}

static inline void tojsonSetBoolean(to_json *aInstance, const char *aFieldName, int *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_boolean;
}

static inline void tojsonSetInteger(to_json *aInstance, const char *aFieldName, int *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_int;
}

static inline void tojsonSetString(to_json *aInstance, const char *aFieldName, const char *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_string;
}

void outputToJsonInitialize(to_json *aOutputToJsons, EjfpFieldVariant *aFieldVariants, size_t aFieldVariantsSize)
{
	const size_t kOutputArraySize = aSize + 2;
	tojsonSetObjectMarkerStart(&outputTojsons[0]);  // Start element, see "mtojson" implementation

	for (size_t i = 1; i < aSize; ++i) {
		const size_t kFieldVariantId = i + 1;
		switch (aFieldVariants[kFieldVariantId].fieldType) {
			case EjfpFieldVariantTypeBoolean:
				tojsonSetBoolean(&aOutputToJsons[i], aFieldVariants[kFieldVariantId].fieldName,
					&aFieldVariants[kFieldVariantId].booleanValue);

				break;

			case EjfpFieldVariantTypeInteger:
				tojsonSetInteger(&aOutputToJsons[i], aFieldVariants[kFieldVariantId].fieldName,
					&aFieldVariants[kFieldVariantId].integerValue);

				break;

			case EjfpFieldVariantTypeString:
				tojsonSetString(&aOutputToJsons[i], aFieldVariants[kFieldVariantId].fieldName,
					&aFieldVariants[kFieldVariantId].stringValue);

				break;
		}
	}
}

size_t ejfpSerialize(EjfpFieldVariant *aFieldVariants, size_t aFieldVariantsSize, char *aOutBuffer,
	size_t aOutBufferSize)
{
	const size_t kOutputArraySize = aFieldVariantsSize + 2;
	to_json outputTojsons[kOutputArraySize] = {{0}};
	outputToJsonInitialize(outputTojsons, aFieldVariants, aFieldVariantsSize);
	const size_t kNSerialized = json_generate(aOutBuffer, outputToJsons, aOutBufferSize);

	return kNSerialized;
}
