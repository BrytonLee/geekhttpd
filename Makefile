DEBUG = -g
CFLAGS = $(DEBUG)
LFLAGS = -lpthread
OBJECTS = main.o config/config.o daemon/daemon.o net/socket.o net/io.o thread/task.o \
		  	process/process.o process/cgi/cgi.o misc/misc.o

geekhttpd : $(OBJECTS)
	cc -o geekhttpd $(LFLAGS) $(OBJECTS)

.PHONY : clean

clean:
	rm geekhttpd $(OBJECTS)
