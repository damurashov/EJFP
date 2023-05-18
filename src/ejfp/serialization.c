//
// serialization.c
//
// Created: 2023-05-18
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#include "ejfp/fieldVariant.h"
#include "ejfp/error.h"
#include <mtojson/mtojson.h>
#include <string.h>

typedef struct {
	struct to_json toJson;

	/// @brief Misc. service fields
	union {
		size_t stringValueLength;
	};
} WrappedToJson;

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
	return aNFields + 1;
}

void outputToJsonInitialize(struct to_json *aOutputToJsons, EjfpFieldVariant *aFieldVariants, size_t aFieldVariantsSize)
{
	const size_t kOutputArraySize = tojsonOutputArraySize(aFieldVariantsSize);
	tojsonSetObjectMarkerStart(&aOutputToJsons[0]);  // Start element, see "mtojson" implementation

	for (size_t i = 0; i < kOutputArraySize; ++i) {
		switch (aFieldVariants[i].fieldType) {
			case EjfpFieldVariantTypeBoolean:
				tojsonSetBoolean(&aOutputToJsons[i], aFieldVariants[i].fieldName,
					&aFieldVariants[i].booleanValue);

				break;

			case EjfpFieldVariantTypeInteger:
				tojsonSetInteger(&aOutputToJsons[i], aFieldVariants[i].fieldName,
					&aFieldVariants[i].integerValue);

				break;

			case EjfpFieldVariantTypeString:
				tojsonSetString(&aOutputToJsons[i], aFieldVariants[i].fieldName,
					aFieldVariants[i].stringValue);

				break;
		}
	}
}

size_t ejfpSerialize(EjfpFieldVariant *aFieldVariants, const size_t aFieldVariantsSize, char *aOutBuffer,
	const size_t aOutBufferSize)
{
	const size_t kOutputArraySize = tojsonOutputArraySize(aFieldVariantsSize);
	struct to_json outputToJsons[kOutputArraySize];
	memset((void *)outputToJsons, 0, kOutputArraySize * sizeof(struct to_json));
	outputToJsonInitialize(outputToJsons, aFieldVariants, aFieldVariantsSize);
	const size_t kNSerialized = json_generate(aOutBuffer, outputToJsons, aOutBufferSize);

	if (kNSerialized == 0) {
		ejfpSetErrorCode(EjfpErrorSerializationNoMemory);
	}

	return kNSerialized;
}
