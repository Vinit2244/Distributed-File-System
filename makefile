main:
	gcc 1.c -o 1.o
	gcc 2.c -o 2.o
	gcc 3.c -o 3.o
	gcc 4.c -o 4.o
	gcc 5.c -o 5.o
clean:
	rm 1.o
	rm 2.o
	rm 3.o
	rm 4.o
	rm 5.o