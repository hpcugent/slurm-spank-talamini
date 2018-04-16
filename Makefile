all: env-test.so

env-test.so: env-test.c
	gcc -std=gnu99 -Wall -o env-test.o -fPIC -c env-test.c
	gcc -shared -o env-test.so env-test.o

clean:
	rm -f env-test.o env-test.so
