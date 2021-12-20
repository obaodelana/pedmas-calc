TARGET = pedmas-calc
BUILDCMD = $(CC) $(TARGET).c -o $(TARGET) -lm -Wall

$(TARGET): $(TARGET).c
	$(BUILDCMD)

debug: $(TARGET).c
	$(BUILDCMD) -DDEBUG_MODE

clean:
	$(RM) $(TARGET)