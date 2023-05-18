#define OHDEBUG_PORT_ENABLE 1
#define OHDEBUG_TAGS_ENABLE "Trace"

#include <OhDebug.hpp>

#include <ejfp/serialization.h>
#include <cstddef>

OHDEBUG_TEST("Serialization: Basic output")
{
	constexpr std::size_t kNFieldVariants = 3;
	constexpr std::size_t kOutputBufferSize = 256;
	char outputBuffer[kOutputBufferSize] = {0};
	EjfpFieldVariant ejfpFieldVariants[kNFieldVariants] {
		{
			EjfpFieldVariantTypeString,
			"string",
			"Some string"
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
	OHDEBUG("Trace", "first", ejfpFieldVariants[0].stringValue);
	std::size_t outputSize = ejfpSerialize(ejfpFieldVariants, kNFieldVariants, outputBuffer, kOutputBufferSize);
	OHDEBUG("Trace", "outputBuffer", outputBuffer, "outputSize", outputSize);
}

int main(void)
{
	OHDEBUG("Trace", "serialization_test");
	OHDEBUG_RUN_TESTS();

	return 0;
}
