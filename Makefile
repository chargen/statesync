statesync: *.c *.h
	gcc -o statesync main.c add.c --std=c99 -ggdb `pkg-config --cflags --libs glib-2.0`
