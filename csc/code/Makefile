fibo:fibo.c main.c
	gcc -c fibo.c -I../include -o fibo.o
	gcc -c main.c -I../include -o main.o
	gcc -o ../fibo fibo.o main.o
clean:
	rm -f fibo.o
	rm -f main.o
