//
// deserialization.c
//
// Created: 2023-05-18
//  Author: Dmitry Murashov (dmtr <DOT> murashov <AT> geoscan.aero)
//

#include "ejfp/ejfp.h"
#include "ejfp/error.h"
#include "ejfp/fieldVariant.h"
#include <jsmn/jsmn.h>
#include <stddef.h>
#include <string.h>

typedef enum {
	BoolFalse = 0,
	BoolTrue,
} Bool;

/// @brief Infers the minimum size of a token array based on how many fields are
/// expected in an incoming JSON
static size_t maxJsmnTokens(size_t aFieldVariantArraySize);

/// @brief  Checks whether JSON structure is supported
Bool jsmntoksIsValid(jsmntok_t *aJsmntoks, size_t aJsmntoksSize, int aNParsedTokens);

/// @brief Sets positions in an input string for input tokens
static inline EjfpError jsmntoksTokenize(Ejfp *aEjfp, jsmntok_t *jsmntoks, size_t jsmntoksSize, const char *aInputBuffer,
	size_t aInputBufferSize);

/// @brief  Converts tokens into values
static inline EjfpError jsmntoksParse(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	jsmntok_t *aJsmntokArray, size_t aJsmntokArraySize);

static inline size_t maxJsmnTokens(size_t aFieldVariantArraySize)
{
	return 1 + aFieldVariantArraySize * 2;
}

/// @brief
/// @param aJsmntoks
/// @param aJsmntoksSize Is expected to be of valid length
/// @return
Bool jsmntoksIsValid(jsmntok_t *aJsmntoks, size_t aJsmntoksSize, int aNParsedTokens)
{
	if (aNParsedTokens != aJsmntoksSize) {
		return BoolFalse;
	}

	if (aJsmntoks[0].type != JSMN_OBJECT) {
		return BoolFalse;
	}

	for (size_t i = 1; i < aNParsedTokens; i += 2) {
		if (aJsmntoks[i].type != JSMN_STRING) {
			return BoolFalse;
		}

		if (aJsmntoks[i + 1].type != JSMN_PRIMITIVE && aJsmntoks[i + 1].type != JSMN_STRING) {
			return BoolFalse;
		}
	}

	return BoolTrue;
}

static inline EjfpError jsmntoksTokenize(Ejfp *aEjfp, jsmntok_t *jsmntoks, size_t jsmntoksSize, const char *aInputBuffer,
	size_t aInputBufferSize)
{
	int error = EjfpOk;
	int nParsedTokens = jsmn_parse(&aEjfp->jsmnParser, aInputBuffer, aInputBufferSize, jsmntoks, jsmntoksSize);

	if (nParsedTokens < 0) {
		switch (nParsedTokens) {
			case JSMN_ERROR_INVAL:  // falls through
				error = EjfpErrorDeserializationInvalidSyntax;

				break;
			case JSMN_ERROR_PART:
				error = EjfpErrorDeserializationPartitioned;

				break;
			case JSMN_ERROR_NOMEM:
				error = EjfpErrorDeserializationNoMemory;

				break;
		}
	} else {
		if (!jsmntoksIsValid(jsmntoks, jsmntoksSize, nParsedTokens)) {  // Verify JSON structure
			error = EjfpErrorDeserializationUnsupportedJsonStructure;
		}
	}

	return error;
}

static inline EjfpError jsmntoksParse(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	jsmntok_t *aJsmntokArray, size_t aJsmntokArraySize)
{
	return EjfpOk;
}

int ejfpDeserialize(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	const char *aInputBuffer, size_t aInputBufferSize)
{
	const size_t jsmntoksSize = maxJsmnTokens(aFieldVariantArraySize);
	jsmntok_t jsmntoks[jsmntoksSize];
	int parsingError = EjfpOk;
	memset(jsmntoks, 0, sizeof(jsmntok_t) * jsmntoksSize);
	parsingError = jsmntoksTokenize(aEjfp, jsmntoks, jsmntoksSize, aInputBuffer, aInputBufferSize);

	if (EjfpOk != parsingError) {
		return parsingError;
	}

	parsingError = jsmntoksParse(aEjfp, aFieldVariantArray, aFieldVariantArraySize, jsmntoks, jsmntoksSize);

	return parsingError;
}
