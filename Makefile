all:
	gcc -g -o printk printk.c

clean:
	rm -f *.o printk
