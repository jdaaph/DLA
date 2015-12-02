CC=mpicxx
CFLAGS=-Wall --pedantic
SRC=diffusion.c
EXECUTABLE=diffusion
LIBS=-L/usr/X11R6/lib -lX11

$(EXECUTABLE): $(SRC)
	$(CC) $(CFLAGS) -o $(EXECUTABLE) $(SRC) $(LIBS)

clean:
	rm -f $(EXECUTABLE)
