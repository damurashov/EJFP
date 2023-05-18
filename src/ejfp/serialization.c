//
// serialization.c
//
// Created: 2023-05-18
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#include "fieldVariant.h"
#include <mtojson/mtojson.h>
#include <string.h>

static void tojsonSetObjectMarkerStart(struct to_json *aInstance);
static void tojsonSetBoolean(struct to_json *aInstance, const char *aFieldName, int *aValue);
static void tojsonSetInteger(struct to_json *aInstance, const char *aFieldName, int *aValue);
static void tojsonSetString(struct to_json *aInstance, const char *aFieldName, const char *aValue);
static size_t tojsonOutputArraySize(size_t aNFields);

static inline void tojsonSetObjectMarkerStart(struct to_json *aInstance)
{
	aInstance->stype = t_to_object;
}

static inline void tojsonSetBoolean(struct to_json *aInstance, const char *aFieldName, int *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_boolean;
}

static inline void tojsonSetInteger(struct to_json *aInstance, const char *aFieldName, int *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_int;
}

static inline void tojsonSetString(struct to_json *aInstance, const char *aFieldName, const char *aValue)
{
	aInstance->name = aFieldName;
	aInstance->value = (void *)aValue;
	aInstance->vtype = t_to_string;
}

static inline size_t tojsonOutputArraySize(size_t aNFields)
{
	return aNFields + 2;
}

void outputToJsonInitialize(struct to_json *aOutputTojsons, EjfpFieldVariant *aFieldVariants, size_t aFieldVariantsSize)
{
	const size_t kOutputArraySize = tojsonOutputArraySize(aFieldVariantsSize);
	tojsonSetObjectMarkerStart(&aOutputTojsons[0]);  // Start element, see "mtojson" implementation

	for (size_t i = 1; i < kOutputArraySize - 1; ++i) {
		const size_t kFieldVariantId = i + 1;
		switch (aFieldVariants[kFieldVariantId].fieldType) {
			case EjfpFieldVariantTypeBoolean:
				tojsonSetBoolean(&aOutputTojsons[i], aFieldVariants[kFieldVariantId].fieldName,
					&aFieldVariants[kFieldVariantId].booleanValue);

				break;

			case EjfpFieldVariantTypeInteger:
				tojsonSetInteger(&aOutputTojsons[i], aFieldVariants[kFieldVariantId].fieldName,
					&aFieldVariants[kFieldVariantId].integerValue);

				break;

			case EjfpFieldVariantTypeString:
				tojsonSetString(&aOutputTojsons[i], aFieldVariants[kFieldVariantId].fieldName,
					aFieldVariants[kFieldVariantId].stringValue);

				break;
		}
	}
}

size_t ejfpSerialize(EjfpFieldVariant *aFieldVariants, const size_t aFieldVariantsSize, char *aOutBuffer,
	const size_t aOutBufferSize)
{
	const size_t kOutputArraySize = tojsonOutputArraySize(aFieldVariantsSize);
	struct to_json outputTojsons[kOutputArraySize];
	memset((void *)outputTojsons, 0, kOutputArraySize * sizeof(struct to_json));
	outputToJsonInitialize(outputTojsons, aFieldVariants, aFieldVariantsSize);
	const size_t kNSerialized = json_generate(aOutBuffer, outputTojsons, aOutBufferSize);

	return kNSerialized;
}
