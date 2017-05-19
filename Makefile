OBJ = linux2.o
SOURCE = linux2.cpp
CC = g++
DEBUG = -g
CFLAGS = -Wall -c $(DEBUG)
LFLAGS = -Wall $(DEBUG)

linux2: $(OBJ)
	$(CC) $(LFLAGS) $(OBJ) -o linux2

linux2.o: $(SOURCE)
	$(CC) $(CFLAGS) $(SOURCE)

clean:
	\rm *.o *~ linux2
