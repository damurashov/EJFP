#define OHDEBUG_PORT_ENABLE 1
#define OHDEBUG_TAGS_ENABLE "Trace"

#include <OhDebug.hpp>

#include <ejfp/deserialization.h>
#include <ejfp/serialization.h>
#include <cstddef>
#include <cstring>

OHDEBUG_TEST("Serialization: Basic output")
{
	Ejfp ejfp{};
	ejfpInitialize(&ejfp);
	constexpr std::size_t kNFieldVariants = 3;
	constexpr std::size_t kOutputBufferSize = 256;
	char outputBuffer[kOutputBufferSize] = {0};
	EjfpFieldVariant ejfpFieldVariants[kNFieldVariants] {
		{
			EjfpFieldVariantTypeString,
			"string",
			0,
		},
		{
			EjfpFieldVariantTypeBoolean,
			"boolean",
			0
		},
		{
			EjfpFieldVariantTypeInteger,
			"integer",
			0
		}
	};
	ejfpFieldVariants[0].stringValue = "Some string";
	OHDEBUG("Trace", "first", ejfpFieldVariants[0].stringValue);
	std::size_t outputSize = ejfpSerialize(&ejfp, ejfpFieldVariants, kNFieldVariants, outputBuffer, kOutputBufferSize);
	OHDEBUG("Trace", "outputBuffer", outputBuffer, "outputSize", outputSize);
}

OHDEBUG_TEST("Deserialization: Basic input")
{
	constexpr const char *input = OHDEBUG_STRINGIFY(
		{
			"message": "Hello",
			"id": 2,
			"activated": true,
			"message 2": "Echo"
		}
	);
	const std::size_t inputLength = strlen(input);
	constexpr std::size_t kNEjfpFieldVariants = 7;
	Ejfp ejfp{};
	EjfpFieldVariant ejfpFieldVariants[kNEjfpFieldVariants] = {};
	ejfpInitialize(&ejfp);
	int error = ejfpDeserialize(&ejfp, ejfpFieldVariants, kNEjfpFieldVariants, input, inputLength);
	OHDEBUG("Trace", "error code:", error);

	for (std::size_t i = 0; i < kNEjfpFieldVariants; ++i) {
		auto type = ejfpFieldVariants[i].fieldType;
		OHDEBUG("Trace", "ejfp type", type);
	}
}

int main(void)
{
	OHDEBUG("Trace", "serialization_test");
	OHDEBUG_RUN_TESTS();

	return 0;
}
