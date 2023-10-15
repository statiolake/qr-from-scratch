.PHONY: all clean

OBJS=main.o gf2.o gf256.o field.o kx.o encoder.o renderer.o painter.o
EXE_NAME=qrcode
CFLAGS=-std=c17 -Wall -Wextra -pedantic -Wmissing-prototypes \
	-Wstrict-prototypes -Wold-style-definition -Werror \
	-g

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS)
	gcc -g -o $(EXE_NAME) $(OBJS)

test_gf256: test_gf256.o field.o gf2.o gf256.o kx.o
	gcc -g -o test_gf256 test_gf256.o field.o gf2.o gf256.o kx.o

.c.o:
	gcc $(CFLAGS) -c $<

clean:
	rm -f *.o
