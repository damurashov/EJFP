#define OHDEBUG_PORT_ENABLE 1
#define OHDEBUG_TAGS_ENABLE "Trace"

#include <ejfp/serialization.h>
#include <OhDebug.hpp>
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
	ejfpSerialize(ejfpFieldVariants, kNFieldVariants, outputBuffer, kOutputBufferSize);
	OHDEBUG("Trace", outputBuffer);
}

int main(void)
{
	OHDEBUG("Trace", "serialization_test");
	OHDEBUG_RUN_TESTS();

	return 0;
}
