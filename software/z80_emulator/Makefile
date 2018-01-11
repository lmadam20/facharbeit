C_SRC_FILES := $(wildcard src/*.c)
CPP_SRC_FILES := $(wildcard src/*.cpp)
C_OBJ_FILES := $(addprefix build/,$(notdir $(C_SRC_FILES:.c=.o)))
CPP_OBJ_FILES := $(addprefix build/,$(notdir $(CPP_SRC_FILES:.cpp=.o)))
LIBS := -lSDL2 -lSDL2_ttf
C_OPTS := -O1

CC = gcc
CPP = g++
MKDIR = mkdir
RMDIR = rm -rf
CP = cp
CD = cd

all: build build/z80emu

build:
	$(MKDIR) build/
	$(CP) font/* build/
	$(CP) README.txt build/
	$(CP) COPYING* build/

build/z80emu: $(C_OBJ_FILES) $(CPP_OBJ_FILES)
	$(CPP) $(C_OPTS) -Wall -o $@ $^ $(LIBS)

build/%.o: src/%.c
	$(CPP) $(C_OPTS) -Iinclude/ -Wall -c $^ -o $@ -DLITTLE_ENDIAN

build/%.o: src/%.cpp
	$(CPP) $(C_OPTS) -Iinclude/ -Wall -c $^ -o $@ -DNO_CYCLING -DLITTLE_ENDIAN

clean:
	$(RMDIR) build/
