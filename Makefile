CC      = tcc
CFLAGS  = -Wall -O2
TARGET  = cal.exe
SRCS    = main.c

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS)

clean:
	rm -f $(TARGET)
