CC			= gcc
CFLAGS		= -g -Wall
LDFLAGS		=
OBJFILES	= myls.o
TARGET		= myls

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~