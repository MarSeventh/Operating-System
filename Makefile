all:test.c
	gcc -o test test.c
run:test
	chmod +x test
	./test
clean:
	rm -f test
