main: main.o fifo.o sem.o spinlock.o tas.o
	gcc -o main main.c fifo.c sem.c spinlock.c tas64.S

main.o:
	gcc -c main.c

fifo.o:
	gcc -c fifo.c

sem.o:
	gcc -c sem.c

spinlock.o:
	gcc -c spinlock.c

tas.o:
	gcc -c tas64.S

clean:
	rm -f main *.o *.stackdump
