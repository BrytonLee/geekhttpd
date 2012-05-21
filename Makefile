OBJECTS = main.o config/config.o daemon/daemon.o net/geekhttpd_socket.o thread_task/thread_task.o

geekhttpd : $(OBJECTS)
	cc -o geekhttpd $(OBJECTS)

.PHONY : clean

clean:
	rm geekhttpd $(OBJECTS)
