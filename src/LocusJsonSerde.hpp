//
// LocusJsonSerde.hpp
//
// Created on: Apr 06, 2023
//     Author: Dmitry Murashov (d.murashov@geoscan.aero)
//

#ifndef WEBAPI_WEBAPI_PRIVATE_LOCUSJSONSERDE_HPP
#define WEBAPI_WEBAPI_PRIVATE_LOCUSJSONSERDE_HPP

#include "utility/OhDebug.hpp"
#include "utility/container/Buffer.hpp"
#include <array>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include <variant>

namespace Web {

/// Stateful serialization/deserialization object. Encapsulates knowledge of
/// locus JSON format for websocket-based message exchange, performs message
/// serialization / deserialization.
class LocusJsonSerde final {
public:
	enum FieldId {
		FieldIdBeacon1Active = 0,
		FieldIdBeacon2Active,
		FieldIdBeacon3Active,
		FieldIdBeacon4Active,
		FieldIdBeacon1PositionX,
		FieldIdBeacon1PositionY,
		FieldIdBeacon1PositionZ,
		FieldIdBeacon2PositionX,
		FieldIdBeacon2PositionY,
		FieldIdBeacon2PositionZ,
		FieldIdBeacon3PositionX,
		FieldIdBeacon3PositionY,
		FieldIdBeacon3PositionZ,
		FieldIdBeacon4PositionX,
		FieldIdBeacon4PositionY,
		FieldIdBeacon4PositionZ,
		FieldIdRequestState,
		FieldIdSetState,
		FieldIdPositionX,
		FieldIdPositionY,
		FieldIdPositionZ,
		FieldIdReceiverId,
		FieldIdRebootEsp32FlashingMode,

		// Meta id.
		FieldIdMax,
	};

	enum class Error {
		// Serialization-related errors
		None = 0,
		Syntax,  // Wrong JSON syntax, or wrong JSON format
		SmallBuffer,
		Unknown,  // Should not pop up
		// Semantic errors
		FieldIdBoundaries,  // Wrong identifier
		UninitializedField,  // Field does not store a value
		// Meta error
		N,
	};

	using FieldVariantType = typename std::variant<std::monostate, bool, int>;

private:
	friend class MtojsonToken;
	friend class MtojsonTokenArray;

	struct Fields : public std::array<FieldVariantType, FieldIdMax> {
		Fields() = default;

		/// (1) Checks `aKey` against the set of fields. (2) Parses `aValue`
		/// and checks its primitive type. If both checks are successful,
		/// updates the field. Otherwise, returns `Error::Syntax`.
		Error trySetFromString(Ut::Ct::ConstCharBuffer aKey, Ut::Ct::ConstCharBuffer aValue);

		inline bool isFieldSet(FieldId aFieldId) const
		{
			if (aFieldId < FieldIdMax) {
				return !std::holds_alternative<std::monostate>(at(aFieldId));
			}

			return false;
		}

		template <class T>
		inline const T *tryAtAs(FieldId aFieldId) const
		{
			OHDEBUG("Trace", "LocusJsonSerde::Fields::tryAtAs", "field id =", static_cast<int>(aFieldId),
				fieldIdAsString(aFieldId));
			if (aFieldId < FieldIdMax && std::holds_alternative<T>(at(aFieldId))) {
				return std::get_if<T>(&at(aFieldId));
			} else {
				return nullptr;
			}
		}

		inline const bool *tryAtAsBoolean(FieldId aFieldId) const
		{
			OHDEBUG("Trace", "LocusJsonSerde::Fields::tryAtAs", "field id =", static_cast<int>(aFieldId),
				fieldIdAsString(aFieldId));

			return tryAtAs<bool>(aFieldId);
		}

		inline const int *tryAtAsInt(FieldId aFieldId) const
		{
			OHDEBUG("Trace", "LocusJsonSerde::Fields::tryAtAs", "field id =", static_cast<int>(aFieldId),
				fieldIdAsString(aFieldId));

			return tryAtAs<int>(aFieldId);
		}

		Error deserialize(const void *aJsonBuffer, std::size_t aJsonBufferSize);
		Error serialize(void *aJsonBuffer, std::size_t aJsonBufferSize) const;
	};

public:
	LocusJsonSerde() = default;
	~LocusJsonSerde() = default;
	void reset();
	static const char *errorAsString(Error aError);

	/// Update state with a JSON object, if it is valid
	inline Error deserialize(const void *aJsonBuffer, std::size_t aJsonBufferSize)
	{
		return fields.deserialize(aJsonBuffer, aJsonBufferSize);
	}

	/// Dump the state in a string form
	inline Error serialize(void *aJsonBuffer, std::size_t aJsonBufferSize) const
	{
		return fields.serialize(aJsonBuffer, aJsonBufferSize);
	}

	template <class I, class T>
	inline void setField(I aFieldId, const T &aFieldValue)
	{
		fields.at(static_cast<FieldId>(aFieldId)) = aFieldValue;
	}

	// TODO: obsolete. Replace w/ `isSetState`
	inline Error tryIsRequestState(bool &aIsRequestState) const
	{
		return tryAs(FieldIdRequestState, aIsRequestState);
	}

	inline Error tryIsSetState(bool &aIsSetState)
	{
		return tryAs(FieldIdSetState, aIsSetState);
	}

	inline bool isSetState() const
	{
		bool isSetState = false;
		tryAs(FieldIdSetState, isSetState);

		return isSetState;
	}

	inline bool isRebootEsp32Bootloader() const
	{
		bool rebootEsp32Bootloader = false;
		tryAs(FieldIdRebootEsp32FlashingMode, rebootEsp32Bootloader);

		return rebootEsp32Bootloader;
	}

	inline bool isRequestState() const
	{
		bool isRequestState = false;
		tryAs(FieldIdRequestState, isRequestState);

		return isRequestState;
	}

	/// Initializes position fields from an array. The layout of the array must
	/// satisfy the following requirements: coordinate step = 1, beacon id step
	/// = 3, i.e. {Beacon 1 X, beacon 1 Y, ..., Beacon 2 X, ...}
	template <class T>
	inline void setPositionFieldsFromArray(T *aArray)
	{
		for (int iField = FieldIdBeacon1PositionX, offset = 0; iField <= FieldIdBeacon4PositionZ; ++iField, ++offset) {
			setField(iField, static_cast<int>(*(aArray + offset)));
		}
	}

	template <class T>
	inline void copyPositionsIfSet(T *aArray) const
	{
		for (int iField = FieldIdBeacon1PositionX, offset = 0; iField <= FieldIdBeacon4PositionZ; ++iField, ++offset) {
			const int *value = fields.tryAtAsInt(static_cast<FieldId>(iField));

			if (value != nullptr) {
				aArray[offset] = static_cast<T>(*value);
			}
		}
	}

	/// Returns "BeaconId", "UninitializedField, or "None" error codes.
	/// `aBeaconId` is expected to be within the range `[FieldIdBeacon<X>Active]_X`
	inline Error tryIsBeaconActive(std::size_t aBeaconId, bool &aIsActive) const
	{
		return tryAs(FieldIdBeacon1Active + aBeaconId, aIsActive, FieldIdBeacon1Active, FieldIdBeacon4Active);
	}

	template <class T, class IntegralType>
	inline Error tryAs(IntegralType aFieldId, T &aValue, const int aBaseFieldId = FieldIdBeacon1Active,
		const int aMaxFieldId = FieldIdMax - 1) const
	{
		OHDEBUG("Trace", "LocusJsonSerde::tryAs", "field id =", static_cast<FieldId>(aFieldId), "field name =",
			fieldIdAsString(static_cast<FieldId>(aFieldId)));
		if (aFieldId < aBaseFieldId || aFieldId > aMaxFieldId) {
			OHDEBUG("Trace", "LocusJsonSerde::tryAs", "Violated field boundaries");

			return Error::FieldIdBoundaries;
		} else {
			const T *value = fields.tryAtAs<T>(static_cast<FieldId>(aFieldId));

			if (value == nullptr) {
				return Error::UninitializedField;
			} else {
				aValue = *value;

				return Error::None;
			}
		}
	}


private:
	/// Converts string field name to `FieldId` identifier. Returns
	/// `FieldIdMax`, if the conversion has failed
	static FieldId stringAsFieldId(const char *aFieldName, std::size_t aFieldNameLength);

	static const char *fieldIdAsString(FieldId aFieldId);

private:
	Fields fields;
};

}  // namespace Web

#endif // WEBAPI_WEBAPI_PRIVATE_LOCUSJSONSERDE_HPP
