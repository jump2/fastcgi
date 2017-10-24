main: main.o fastcgi.o config.o httpd.o
	gcc -o main main.o fastcgi.o config.o httpd.o
main.o: main.c fastcgi.h config.h httpd.h
	gcc -c main.c   
fastcgi.o: fastcgi.c fastcgi.h
	gcc -c fastcgi.c
config.o: config.c config.h
	gcc -c config.c
httpd.o: httpd.c httpd.h
	gcc -c httpd.c
clean:
	rm -rf *.o main 
