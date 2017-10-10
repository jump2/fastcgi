main: main.o fastcgi.o config.o
	gcc -o main main.o fastcgi.o config.o
main.o: main.c fastcgi.h config.h
	gcc -c main.c   
fastcgi.o: fastcgi.c fastcgi.h   
	gcc -c fastcgi.c
config.o: config.c config.h
	gcc -c config.c
clean:
	rm -rf *.o main 
