LIBDIR = -levent

GCC = gcc
GCCFLAGS = -g -finline-functions -Wall -Winline -pipe

TARGET = ev-server
OBJS   = event-http.o util.o

$(TARGET) : $(OBJS)
	$(GCC) -g -o $@ $^ $(LIBDIR)

%.o : %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)

cleanobj:
	rm -f $(OBJS)
