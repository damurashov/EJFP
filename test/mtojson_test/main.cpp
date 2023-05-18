#define OHDEBUG_PORT_ENABLE 1
#define OHDEBUG_TAGS_ENABLE "Trace"

#include <OhDebug.hpp>
#include <mtojson/mtojson.h>

OHDEBUG_TEST("String serialization")
{
	char *string = "Hello";
	std::size_t stringLength = 5;
	struct to_json toJson[2] {};
	toJson[0].value = (void *)string;
	toJson[0].vtype = t_to_string;
	toJson[0].stype = t_to_object;
	toJson[0].name = "String";
	char outputBuffer[256] = {0};
	json_generate(outputBuffer, &toJson[0], 256);
	OHDEBUG("Trace", outputBuffer);
}

int main(void)
{
	OHDEBUG("Trace", "mtojson_test");
	OHDEBUG_RUN_TESTS();

	return 0;
}
