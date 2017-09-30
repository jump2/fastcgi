main: main.o fastcgi.o
	gcc -o main main.o fastcgi.o
main.o: main.c fastcgi.h
	gcc -c main.c   
fastcgi.o: fastcgi.c fastcgi.h   
	gcc -c fastcgi.c
clean:
	rm -rf *.o main 
