.PHONY: all clean

OBJS=main.o encoder.o renderer.o painter.o
EXE_NAME=qrcode
CFLAGS=-std=c17 -Wall -Wextra -pedantic -Wmissing-prototypes \
	-Wstrict-prototypes -Wold-style-definition -Werror

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS)
	gcc -o $(EXE_NAME) $(OBJS)

.c.o:
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o
