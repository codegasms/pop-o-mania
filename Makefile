CC = g++ -DSUPPORT_FILEFORMAT_PNG -DSUPPORT_FILEFORMAT_BMP -DSUPPORT_FILEFORMAT_JPG
cflags = -std=c++17 -O2
cflags += -Ivendor/raylib/src -Iinclude -Ivendor/zip/src -Iinclude/sound -I/usr/x86_64-w64-mingw32/include/SDL2 -I/usr/include/SDL2 -D_REENTRANT
ldflags += -lstdc++ -Lvendor/raylib/src -lraylib -Lvendor/zip/build -lzip -lpthread -ldl -lSDL2main -lSDL2 -lpthread
name = pop-o-mania

sources = $(wildcard src/*.cpp)
objects = $(patsubst src/%, object/%,$(sources:.cpp=.o))
deps = $(objects:.o=.d)

-include $(deps)
.PHONY: all clean

all: files deps $(name)

files:
	mkdir -p bin object

deps:
	cd vendor/raylib/src/ && $(MAKE)
	cd vendor/zip && mkdir -p build && cd build && cmake .. -DCMAKE_DISABLE_TESTING=1 && make -j12

run: $(name)
	mkdir -p beatmaps
	bin/$(name)

$(name): $(objects)
	$(CC) -o bin/$(name) $^ $(ldflags)

object/%.o: src/%.cpp
	$(CC) -MMD -o $@ -c $< $(cflags)

clean:
	rm -rf vendor/raylib/src/*.o vendor/raylib/src/libraylib.a bin/$(name) $(objects) object/*.d vendor/zip/build SDL_mixer/build vendor/raylib/src/external/glfw/build
