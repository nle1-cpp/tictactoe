ifeq ($(OS),Windows_NT)
	PLATFORM = Windows
	EXE = .exe
	RM = del /Q
	DLL = "C:/Progra~1/mosquitto/libmosquitto.dll"
	INCLUDES = -I"C:/Progra~1/mosquitto/devel"
	LIBS = -L"C:/Progra~1/mosquitto/devel" -lmosquitto
else
	PLATFORM := $(shell uname)
	EXE =
	RM = rm -f
	INCLUDES = -I/usr/include
	LIBS = -lmosquitto
endif

CC = gcc
TARGET = player$(EXE)

all: $(TARGET)

$(TARGET): player.c
	$(CC) player.c $(INCLUDES) $(LIBS) -o $(TARGET)
ifeq ($(PLATFORM),Windows)
	# Copy DLL to current folder if it exists
	@cmd /C "if exist $(DLL) copy \"$(DLL)\" ."
endif

run:
ifeq ($(PLATFORM),Windows)
	./$(TARGET)
else
	LD_LIBRARY_PATH=/usr/lib ./$(TARGET)
endif

clean:
	$(RM) $(TARGET)
ifeq ($(PLATFORM),Windows)
	$(RM) libmosquitto.dll
endif
