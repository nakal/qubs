
OBJS=qubs.o bdd.o list.o hash.c memory.o

all:
	$(MAKE) compile
	$(CC) -o qubs $(OBJS)

compile: $(OBJS)

.c.o:
	$(CC) -c -o $@ $<

clean:
	rm -f *.o qubs
