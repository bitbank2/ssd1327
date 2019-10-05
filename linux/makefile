CFLAGS=-c -Wall -O2 -I../src
LIBS = -lm -lpthread

all: libssd1327.a

libssd1327.a: ssd1327.o ../src/ssd1327.h ../src/ssd1327.cpp
	ar -rc libssd1327.a ssd1327.o ;\
	sudo cp libssd1327.a /usr/local/lib ;\
	sudo cp ../src/ssd1327.h /usr/local/include

ssd1327.o: ssd1327.c ../src/ssd1327.cpp ../src/ssd1327.h
	$(CC) $(CFLAGS) -D_LINUX_ ssd1327.c

clean:
	rm *.o libssd1327.a
