//
// LocusJsonSerde.cpp
//
// Created on: Apr 06, 2023
//     Author: Dmitry Murashov (d.murashov@geoscan.aero)
//

#include "utility/algorithm/Algorithm.hpp"
#include <jsmn/jsmn.h>
#include <mtojson/mtojson.h>
#include <algorithm>
#include <cstring>

#include "LocusJsonSerde.hpp"

namespace Web {

class JsmnToken;
class JsmnTokenArray;
static LocusJsonSerde::Error fieldVariantTrySetFromBoolString(LocusJsonSerde::FieldVariantType &,
	const Ut::Ct::ConstCharBuffer &aInput);
static LocusJsonSerde::Error fieldVariantTrySetFromIntString(LocusJsonSerde::FieldVariantType &,
	const Ut::Ct::ConstCharBuffer &aInput);

/// Parser function. Expects `aString` to be of certain format.
/// `aString` does not necessarily have null-termination.
using FieldParsingCallable = LocusJsonSerde::Error(*)(LocusJsonSerde::FieldVariantType &, const Ut::Ct::ConstCharBuffer &aString);

constexpr std::size_t maxJsmnTokens()
{
	// object type token + `FieldIdMax` * (key + value) pairs
	return 1 + LocusJsonSerde::FieldIdMax * 2;
}

/// Some operations require creating a temporary buffer at runtime. This
/// constexpr defines its max. length.
constexpr std::size_t maxTempBufferSize ()
{
	return 1024;
}

constexpr std::size_t maxBeacons()
{
	return 4;
}

static constexpr std::array<const char *, LocusJsonSerde::FieldIdMax> kFieldIdStringMap {{
	// TODO: add "status poll" command
	"is1Active",
	"is2Active",
	"is3Active",
	"is4Active",
	"position1x",
	"position1y",
	"position1z",
	"position2x",
	"position2y",
	"position2z",
	"position3x",
	"position3y",
	"position3z",
	"position4x",
	"position4y",
	"position4z",
	"isRequestState",
	"isSetState",
	"positionX",
	"positionY",
	"positionZ",
	"receiverId",
	"isEsp32Flash",
}};

static constexpr std::array<FieldParsingCallable, LocusJsonSerde::FieldIdMax> kFieldIdParserMap {{
	// Active
	fieldVariantTrySetFromBoolString,
	fieldVariantTrySetFromBoolString,
	fieldVariantTrySetFromBoolString,
	fieldVariantTrySetFromBoolString,

	// Position
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,
	fieldVariantTrySetFromIntString,

	fieldVariantTrySetFromBoolString,  // "Request state"
	fieldVariantTrySetFromBoolString,  // "is set state"
	fieldVariantTrySetFromIntString,  // "x"
	fieldVariantTrySetFromIntString,  // "y"
	fieldVariantTrySetFromIntString,  // "z"
	fieldVariantTrySetFromIntString,  // "Receiver identifier"
	fieldVariantTrySetFromBoolString,  // "is ESP32 flashing mode"
}};

struct JsmnToken : jsmntok_t {
	Ut::Ct::ConstCharBuffer asConstCharBuffer(const char *aInputBuffer) const
	{
		return {&aInputBuffer[start], end - start};
	}
};

/// Encapsulation of an array of parsed tokens
struct JsmnTokenArray : std::array<JsmnToken, maxJsmnTokens()> {
	std::size_t nParsedTokens;

	JsmnTokenArray() :
		std::array<JsmnToken, maxJsmnTokens()>{},
		nParsedTokens{parsedTokensUninitialized()}
	{
	}

	/// Iterates over the array, treating it as a POD object. The iteration
	/// continues for as long as `aCallableReturns` true.
	///
	/// \pre The array must be valid. See `isValid()`. Otherwise, the
	/// method does not make sense.
	template <class KeyValueConstCharBuffersAcceptingCallable>
	void iterAsKvPairsWhile(const char *aParsedBuffer, KeyValueConstCharBuffersAcceptingCallable &&aCallable)
	{
		if (isInitialized()) {
			for (std::size_t i = 1; i < nParsedTokens; i += 2) {
				if (!aCallable(at(i).asConstCharBuffer(aParsedBuffer), at(i + 1).asConstCharBuffer(aParsedBuffer))) {
					break;
				}
			}
		}
	}

	constexpr std::size_t parsedTokensUninitialized() const
	{
		return size() + 1;
	}

	inline bool isInitialized() const
	{
		return parsedTokensUninitialized() != nParsedTokens;
	}

	/// Checks whether it conforms the following structure
	///
	/// ```
	/// {
	///   STRING KEY: NUMBER (INT, FLOAT, OR BOOL),
	///   ...
	/// }
	/// ```
	bool isValid() const
	{
		constexpr std::size_t kMinValidLength = 1;

		if (!isInitialized()) {
			return false;
		}

		if (nParsedTokens < kMinValidLength) {
			return false;
		}

		if (at(0).type != jsmntype_t::JSMN_OBJECT) {
			return false;
		}

		for (std::size_t i = 1; i < nParsedTokens; i += 2) {
			if (at(i).type != jsmntype_t::JSMN_STRING) {
				return false;
			}

			if (at(i + 1).type != jsmntype_t::JSMN_PRIMITIVE) {
				return false;
			}
		}

		return true;
	}

	LocusJsonSerde::Error parseUpdate(const void *aBuffer, std::size_t aBufferSize)
	{
		jsmn_parser jsmnParser;
		jsmn_init(&jsmnParser);
		const auto parseResult = jsmn_parse(&jsmnParser, static_cast<const char *>(aBuffer), aBufferSize, data(),
			size());

		if (parseResult < 0) {
			switch (static_cast<jsmnerr>(parseResult)) {
				case jsmnerr::JSMN_ERROR_INVAL:  // falls through
				case jsmnerr::JSMN_ERROR_PART:
					return LocusJsonSerde::Error::Syntax;
				case jsmnerr::JSMN_ERROR_NOMEM:
					return LocusJsonSerde::Error::SmallBuffer;
				default:
					return LocusJsonSerde::Error::Unknown;
			}
		} else {
			nParsedTokens = static_cast<std::size_t>(parseResult);

			if (isValid()) {  // Verify JSON structure
				return LocusJsonSerde::Error::None;
			} else {
				return LocusJsonSerde::Error::Syntax;
			}
		}
	}
};

struct MtojsonToken : to_json {
	/// When creating a structure, "mtojson" seems to expect the first object
	/// in an array to specify the structure's type.
	inline void setObjectMarkerStart()
	{
		stype = t_to_object;
	}

	/// `aPersistentValue` is expected to stay valid until `json_generate` is invoked
	inline void setBoolean(const char *aNullTerminatedPersistentFieldName, const bool *aPersistentValue)
	{
		name = aNullTerminatedPersistentFieldName;
		value = static_cast<const void *>(aPersistentValue);
		vtype = t_to_boolean;
	}

	/// `aPersistentValue` is expected to stay valid until `json_generate` is invoked
	inline void setInteger(const char *aNullTerminatedPersistentFieldName, const int *aPersistentValue)
	{
		name = aNullTerminatedPersistentFieldName;
		value = static_cast<const void *>(aPersistentValue);
		vtype = t_to_int;
	}

	/// Tries to set the instance with an appropriate value. Returns true, if
	/// `aFieldVariant` actually contains a value. False otherwise.
	///
	/// \pre `aFieldVariant` is expected to be initialized with the correct
	/// type in the first place.
	inline bool trySetValueWithFieldVariant(LocusJsonSerde::FieldId aFieldId,
		const LocusJsonSerde::FieldVariantType &aFieldVariant)
	{
		return std::visit(Ut::Al::makeVisitor(
			[this, aFieldId](const bool &aValue)
			{
				setBoolean(LocusJsonSerde::fieldIdAsString(aFieldId), &aValue);

				return true;
			},
			[this, aFieldId](const int &aValue)
			{
				setInteger(LocusJsonSerde::fieldIdAsString(aFieldId), &aValue);

				return true;
			},
			[](...)
			{
				return false;
			}),
			aFieldVariant);
	}

	inline MtojsonToken() :
		to_json{
			.name = nullptr,
			.value = nullptr,
			.count = nullptr,
			.stype = static_cast<json_to_type>(0),
			.vtype = static_cast<json_to_type>(0),
		}
	{
	}
};

/// Encapsulation of "mtojson" token array, integrated w/
/// `LocusJsonSerde::Fields`
struct MtojsonTokenArray : public std::array<MtojsonToken, LocusJsonSerde::FieldIdMax> {

	MtojsonTokenArray() :
		std::array<MtojsonToken, LocusJsonSerde::FieldIdMax>{{{}}}
	{
	}

	std::size_t initFromFields(const LocusJsonSerde::Fields &aFields)
	{
		at(0).setObjectMarkerStart();
		std::size_t iToken = 0;

		for (auto iField = 0; iField < LocusJsonSerde::FieldIdMax; ++iField) {
			if (at(iToken).trySetValueWithFieldVariant(static_cast<LocusJsonSerde::FieldId>(iField),
					aFields.at(static_cast<LocusJsonSerde::FieldId>(iField)))) {
				++iToken;
			}
		}

		return iToken;
	}
};

static inline LocusJsonSerde::Error fieldVariantTrySetFromBoolString(LocusJsonSerde::FieldVariantType &aFieldVariant,
	const Ut::Ct::ConstCharBuffer &aConstCharBuffer)
{
	if (0 == strncmp("true", aConstCharBuffer.data(), aConstCharBuffer.size())) {
		aFieldVariant = true;

		return LocusJsonSerde::Error::None;
	} else if (0 == strncmp("false", aConstCharBuffer.data(), aConstCharBuffer.size())) {
		aFieldVariant = false;

		return LocusJsonSerde::Error::None;
	} else {
		return LocusJsonSerde::Error::Syntax;
	}
}

static inline LocusJsonSerde::Error fieldVariantTrySetFromIntString(LocusJsonSerde::FieldVariantType &aFieldVariant,
	const Ut::Ct::ConstCharBuffer &aInput)
{
	char *end = nullptr;
	const int conversionResult = strtol(aInput.data(), &end, 10);

	if (end == nullptr) {
		return LocusJsonSerde::Error::Syntax;
	} else {
		aFieldVariant = conversionResult;

		return LocusJsonSerde::Error::None;
	}
}

void LocusJsonSerde::reset()
{
	for (auto &field : fields) {
		field = FieldVariantType{};
	}
}

const char *LocusJsonSerde::errorAsString(Error aError)
{
	static constexpr std::array<const char *, static_cast<std::size_t>(LocusJsonSerde::Error::N)> kErrorStringMap = {{
		"",
		"JSON syntax",
		"Small buffer",
		"Unknown error",
		"Wrong field identifier",
		"Uninitialized field",
	}};

	return kErrorStringMap[static_cast<std::size_t>(aError)];
}

LocusJsonSerde::FieldId LocusJsonSerde::stringAsFieldId(const char *aFieldName, std::size_t aFieldNameLength)
{
	int fieldId = 0;

	for (const auto &fieldIdMapEntry : kFieldIdStringMap) {
		if (strncmp(fieldIdMapEntry, aFieldName, aFieldNameLength) == 0) {
			break;
		}

		++fieldId;
	}

	return static_cast<FieldId>(fieldId);
}

const char *LocusJsonSerde::fieldIdAsString(LocusJsonSerde::FieldId aFieldId)
{
	if (aFieldId < LocusJsonSerde::FieldIdMax) {
		return kFieldIdStringMap[aFieldId];
	} else {
		return "fieldIdAsString: WRONG FIELD ID";  // Should not reach here. Plant a distinguishable marker
	}
}

LocusJsonSerde::Error LocusJsonSerde::Fields::trySetFromString(Ut::Ct::ConstCharBuffer aKey,
	Ut::Ct::ConstCharBuffer aValue)
{
	if (const auto fieldId = stringAsFieldId(aKey.data(), aKey.size()); fieldId < FieldIdMax) {
		const auto parse = kFieldIdParserMap[fieldId];
		const auto valueParsingError = parse(at(fieldId), aValue);

		return valueParsingError;
	} else {
		return Error::Syntax;  // Unexpected field
	}
}

LocusJsonSerde::Error LocusJsonSerde::Fields::deserialize(const void *aJsonBuffer, std::size_t aJsonBufferSize)
{
	JsmnTokenArray jsmnTokenArray{};
	auto parsingError = jsmnTokenArray.parseUpdate(aJsonBuffer, aJsonBufferSize);

	if (parsingError == LocusJsonSerde::Error::None) {
		jsmnTokenArray.iterAsKvPairsWhile(static_cast<const char *>(aJsonBuffer),
			[this, &parsingError](const Ut::Ct::ConstCharBuffer &aKey, const Ut::Ct::ConstCharBuffer &aValue) mutable
			{
				const auto fieldSettingError{trySetFromString(aKey, aValue)};

				if (fieldSettingError != LocusJsonSerde::Error::None) {
					parsingError = fieldSettingError;

					return false;
				}

				return true;  // Continue iteration
			});
	}

	return parsingError;
}

LocusJsonSerde::Error LocusJsonSerde::Fields::serialize(void *aJsonBuffer, std::size_t aJsonBufferSize) const
{
	MtojsonTokenArray mtoJsonTokenArray{};
	mtoJsonTokenArray.initFromFields(*this);
	const auto jsonLength = json_generate(static_cast<char *>(aJsonBuffer), mtoJsonTokenArray.data(), aJsonBufferSize);

	if (jsonLength == 0) {
		return Error::SmallBuffer;
	} else {
		return Error::None;
	}
}

}  // namespace Web
