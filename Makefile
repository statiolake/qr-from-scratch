.PHONY: all clean

OBJS=main.o encoder.o renderer.o
EXE_NAME=qrcode

all: $(EXE_NAME)

$(EXE_NAME): $(OBJS)
	gcc -o $(EXE_NAME) $(OBJS)

.c.o:
	gcc -c $<

clean:
	rm -f *.o
