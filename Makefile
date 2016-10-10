TARGET=countTiles
SOURCE=countTiles.cpp
CXX=clang++

# virtualをなくすと速くなる
EXTRA_CPPFLAGS=-DDISABLE_VIRTUAL
CPPFLAGS=-std=c++14 -Wall -O2 $(EXTRA_CPPFLAGS)
LIBS=
LDFLAGS=

.PHONY: all check clean rebuild

all: $(TARGET)

$(TARGET): $(SOURCE) Makefile
	$(CXX) $(CPPFLAGS) -o $@ $< $(LIBS) $(LDFLAGS)
	./$(TARGET) | grep : | wc | grep " 93600 "
	time ./$(TARGET) > /dev/null

check:
	@./$(TARGET)

clean:
	$(RM) $(TARGET)

rebuild: clean all
