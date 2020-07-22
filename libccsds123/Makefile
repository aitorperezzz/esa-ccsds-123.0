# Compiler and linker.
CC=gcc

# Name of the library.
TARGET=libccsds123.so

# Compiler flags.
CCFLAGS=-I./inc -fPIC

# Linker flags.
LDFLAGS=-shared

# List all the sources needed for this project.
SOURCES=$(wildcard src/*.c)

# List the objects needed for the project (from sources).
OBJECTS=$(patsubst %.c,%.o,$(SOURCES))

# List dependency files, one for each source file.
DEPENDS=$(patsubst %.c,%.d,$(SOURCES))

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

-include $(DEPENDS)

%.o: %.c Makefile
	$(CC) $(CCFLAGS) -DNO_COMPUTE_LOCAL -MMD -MP -c $< -o $@

clean:
	-rm -f $(OBJECTS) $(DEPENDS) $(TARGET)
