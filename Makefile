statesync: *.c *.h
	clang *.c -o statesync --std=c99 -ggdb `pkg-config --cflags --libs glib-2.0` -lcrypto

statesync-gcc: *.c *.h
	gcc -o statesync-gcc *.c --std=c99 -ggdb `pkg-config --cflags --libs glib-2.0` -lcrypto

statesync-xx:
	g++ -o statesync-gcc *.c -ggdb `pkg-config --cflags --libs glib-2.0` -lcrypto

clean:
	rm statesync
