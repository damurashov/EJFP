# Embedded JSON Field Protocol

EJFP is a neat little library for serializing and deserializing JSON, relying
on 2 other neat little libraries: "jsmn" and "mtojson". It is small, efficient,
`malloc`-free, self-contained (drop-n-use), and primarily targets embedded
applications.

Once you accept a set of limitations, it becomes a one-stop solution for
serializing and deserializing JSON. The set of limitations is as follows:

- The library is not thread-safe (YET);
- The library is stateless (YET), meaning that you cannot feed it a chunk of
  JSON, and then the next one later;
- The library does not separate between JSON objects (YET);
- The library only treats JSON objects with integers, strings, booleans,
  floats, and `null`s, i.e. JSON structures of the following format:

```json
{
	"String field": "String value",
	"Integer field": 42,
	"Float field": 1E-3,
	"Null field": null,
	"Boolean field": true,
	"Another string field": "Another string value",
	...
}
```

# TODO

- Fully stateful deserialization;
- Inter-backend compatibility: `null` values;
- CMake-based build system;
- Make-based build system;
- Array support;
- Arbitrary JSON format support (long shot);
