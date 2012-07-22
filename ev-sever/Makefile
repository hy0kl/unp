LIBDIR = -levent

GCC = gcc
GCCFLAGS = -g -finline-functions -Wall -Winline -pipe

TARGET = ev-server
OBJS   = event-http.o

$(TARGET) : $(OBJS)
	$(GCC) -g -o $@ $^ $(LIBDIR)

%.o : %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS)
