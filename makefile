OPTIONS=-Wextra -Wall -Werror -pedantic -std=c99 -Wformat=2 -Wformat-truncation -Wundef -Wdouble-promotion -Wshadow -Wpointer-arith -Wcast-align -Wconversion -Wfloat-conversion -Wsign-conversion
DISABLEDWARNINGS=-Wno-unused-variable -Wno-unused-function -Wno-unused-parameter -Wno-missing-field-initializers -Wno-format-nonliteral
OPTIMIZATIONS=-O3

LIBRARIES=
INCLUDES=
CODEFLAGS=
FILE_MAIN_CODE=
FILE_MAIN_OUTPUT=

UNAME_S = $(shell uname -s)
ifeq ($(OS),WINDOWS_NT)
	FILE_MAIN_CODE+=win32_main.c
	FILE_MAIN_OUTPUT+=win32_main.exe
else ifeq ($(UNAME_S),Linux)
	FILE_MAIN_CODE+=linux_main.c
	FILE_MAIN_OUTPUT+=linux_main
	LIBRARIES+=-ltls
else
	$(error OS not supported by this makefile)
endif

build:
	gcc $(FILE_MAIN_CODE) -o $(FILE_MAIN_OUTPUT) $(OPTIONS) $(DISABLEDWARNINGS) $(OPTIMIZATIONS) $(CODEFLAGS) $(LIBRARIES) $(INCLUDES)

run:
	./$(FILE_MAIN_OUTPUT)
