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
#include <stdlib.h>
#include <string.h>

#ifndef OHDEBUG
#define OHDEBUG(...)
#endif

typedef enum {
	BoolFalse = 0,
	BoolTrue,
} Bool;

/// @brief Infers the minimum size of a token array based on how many fields are
/// expected in an incoming JSON
static size_t maxJsmnTokens(size_t aFieldVariantArraySize);

/// @brief  Checks whether JSON structure is supported
Bool jsmntoksIsValid(jsmntok_t *aJsmntoks, int aNParsedTokens);

/// @brief Sets positions in an input string for input tokens
static EjfpError jsmntoksTokenize(Ejfp *aEjfp, jsmntok_t *jsmntoks, size_t *jsmntoksSize, const char *aInputBuffer,
	size_t aInputBufferSize);

/// @brief  Converts tokens into values
static EjfpError jsmntoksParse(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	jsmntok_t *aJsmntokArray, size_t aJsmntokArraySize, const char *aInputBuffer);

static int intMin(int aLhs, int aRhs)
{
	return aLhs > aRhs ? aRhs : aLhs;
}

/// @brief "jsmn" does not make distinctions between integers, floats, and
/// booleans
/// @pre The type must be `JSMN_PRIMITIVE`
EjfpFieldVariantType jsmntokGetRefinedPrimitiveType(jsmntok_t *aJsmntok)
{
}

static inline size_t maxJsmnTokens(size_t aFieldVariantArraySize)
{
	return 1 + aFieldVariantArraySize * 2;
}

/// @brief
/// @param aJsmntoks
/// @param aJsmntoksSize Is expected to be of valid length
/// @return
Bool jsmntoksIsValid(jsmntok_t *aJsmntoks, int aNParsedTokens)
{
	size_t i = 0;

	if (aJsmntoks[i].type == JSMN_OBJECT) {
		++i;
	}

	for (; i < aNParsedTokens; i += 2) {
		if (aJsmntoks[i].type != JSMN_STRING) {
			return BoolFalse;
		}

		if (aJsmntoks[i + 1].type != JSMN_PRIMITIVE && aJsmntoks[i + 1].type != JSMN_STRING) {
			return BoolFalse;
		}
	}

	return BoolTrue;
}

/// @brief
/// @param aEjfp
/// @param jsmntoks
/// @param jsmntoksSize  Will be set to the actual number of parsed tokens
/// @param aInputBuffer
/// @param aInputBufferSize
/// @return
static inline EjfpError jsmntoksTokenize(Ejfp *aEjfp, jsmntok_t *jsmntoks, size_t *jsmntoksSize, const char *aInputBuffer,
	size_t aInputBufferSize)
{
	int error = EjfpOk;
	int nParsedTokens = jsmn_parse(&aEjfp->jsmnParser, aInputBuffer, aInputBufferSize, jsmntoks, *jsmntoksSize);

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
		if (!jsmntoksIsValid(jsmntoks, nParsedTokens)) {  // Verify JSON structure
			error = EjfpErrorDeserializationUnsupportedJsonStructure;
		}

		*jsmntoksSize = nParsedTokens;
	}

	return error;
}

/// @brief Expects a sequence of ("key": true | false | null | INTEGER | FLOAT) pairs
static inline EjfpError jsmntoksParse(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	jsmntok_t *aJsmntokArray, size_t aJsmntokArraySize, const char *aInputBuffer)
{
	size_t iFieldVariant = 0;
	static const size_t kStartingPosition = 1;

	for (jsmntok_t *token = &aJsmntokArray[kStartingPosition]; token < aJsmntokArray + aJsmntokArraySize;
			++token, ++iFieldVariant) {
		// Handle the case where there is not enough instances in the "Field variant" array
		if (iFieldVariant == aFieldVariantArraySize) {
			return EjfpErrorDeserializationNoMemory;
		}

		// Initialize field name
		size_t tokenLength = token->end - token->start;
		const char *tokenStart = &aInputBuffer[token->start];
		const char *tokenEnd = &aInputBuffer[token->end];
		aFieldVariantArray[iFieldVariant].fieldName = tokenStart;
		aFieldVariantArray[iFieldVariant].fieldNameLength = tokenLength;

		// Advance the token
		++token;
		tokenStart = &aInputBuffer[token->start];
		tokenEnd = &aInputBuffer[token->end];
		tokenLength = token->end - token->start;

		{
			switch (token->type) {
				case JSMN_STRING:
					aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeString;
					aFieldVariantArray[iFieldVariant].stringValue = tokenStart;
					aFieldVariantArray[iFieldVariant].stringValueLength = tokenLength;

					break;

				// "jsml" does not make a distinction b/w integer, null, float, and boolean types
				case JSMN_PRIMITIVE: {
					static const char *trueValue = "true";
					static const char *falseValue = "false";
					static const char *nullValue = "null";
					static const size_t trueValueLength = sizeof("true");
					static const size_t falseValueLength = sizeof("false");
					static const size_t nullValueLength = sizeof("null");

					// Check booleans
					if (strncmp(tokenStart, trueValue, intMin(tokenLength, trueValueLength)) == 0) {
						aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeBoolean;
						aFieldVariantArray[iFieldVariant].booleanValue = BoolTrue;
					} else if (strncmp(tokenStart, falseValue, intMin(tokenLength, falseValueLength)) == 0) {
						aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeBoolean;
						aFieldVariantArray[iFieldVariant].booleanValue = BoolFalse;
					// Check null
					} else if (strncmp(tokenStart, nullValue, intMin(tokenLength, nullValueLength)) == 0) {
						aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeNull;
					// Check numeric
					} else {
						// Temporary storage to provide null-terminated strings
						aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeInteger;  // Assume integer by default

						// Check whether it is a float through looking for special characters unique to float format
						enum {
							FloatMarkerNone = 0,
							FloatMarkerDot = '.',
							FloatMarkerEBig = 'E',
							FloatMarkerESmall = 'e'
						} floatMarker = FloatMarkerNone;

						for (const char *ch = tokenStart; ch != tokenEnd; ++ch) {
							if (*ch == FloatMarkerDot || *ch == FloatMarkerEBig || *ch == FloatMarkerESmall) {
								aFieldVariantArray[iFieldVariant].fieldType = EjfpFieldVariantTypeFloat;

								break;
							}
						}

						switch (aFieldVariantArray[iFieldVariant].fieldType) {
							case EjfpFieldVariantTypeInteger:
								aFieldVariantArray[iFieldVariant].integerValue = atoi(tokenStart);

								break;

							case EjfpFieldVariantTypeFloat:
								aFieldVariantArray[iFieldVariant].floatValue = atof(tokenStart);

								break;
						}
					}

					break;
				}
			}
		}

	}

	return EjfpOk;
}

int ejfpDeserialize(Ejfp *aEjfp, EjfpFieldVariant *aFieldVariantArray, size_t aFieldVariantArraySize,
	const char *aInputBuffer, size_t aInputBufferSize)
{
	size_t jsmntoksSize = maxJsmnTokens(aFieldVariantArraySize);
	jsmntok_t jsmntoks[jsmntoksSize];
	int parsingError = EjfpOk;
	memset(jsmntoks, 0, sizeof(jsmntok_t) * jsmntoksSize);
	parsingError = jsmntoksTokenize(aEjfp, jsmntoks, &jsmntoksSize, aInputBuffer, aInputBufferSize);

	if (EjfpOk != parsingError) {
		return parsingError;
	}

	switch (parsingError) {
		case EjfpOk:
		case EjfpErrorDeserializationNoMemory:
			break;

		default:
			return parsingError;
	}

	parsingError = jsmntoksParse(aEjfp, aFieldVariantArray, aFieldVariantArraySize, jsmntoks, jsmntoksSize,
		aInputBuffer);

	if (EjfpOk == parsingError) {

	}

	return parsingError;
}
