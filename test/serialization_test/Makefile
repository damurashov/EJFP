EXECUTABLE = build/serialization_test

all: $(EXECUTABLE)

$(EXECUTABLE): build
	$(MAKE) -C build

build:
	mkdir -p build && \
		cd build && \
		cmake ..

run: $(EXECUTABLE)
	$(EXECUTABLE)

.PHONY: $(EXECUTABLE)

clean:
	rm -rf build
	rm -rf *txt.user
