# Compiler and linker.
CC=g++

# Name of the target executable.
TARGET=test

# Compiler flags.
CCFLAGS=-g -Wall -I./libccsds123/inc

# Paths for the libraries needed at linking time 
# (library should be compiled and available there).
LIBPATHS=-L./libccsds123

# Specific library name.
LDFLAGS=-l:libccsds123.so

# Objects needed for the project.
OBJECTS=main.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(CCFLAGS) $(OBJECTS) -o $(TARGET) $(LIBPATHS) $(LDFLAGS)

main.o: main.cpp Makefile libccsds123/inc/compress_ccsds123.h libccsds123/inc/decompress_ccsds123.h
	$(CC) $(CCFLAGS) -c -o main.o main.cpp

clean:
	-rm -f $(TARGET) $(OBJECTS)
