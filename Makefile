
SOURCES=env-test.c pbs_nodefile.c
OBJECTS=$(SOURCES:.c=.o)
SHLIBS=$(OBJECTS:.o=.so)

CC=gcc
CFLAGS=-std=gnu99 -Wall -fPIC
LDFLAGS=-shared

all: $(OBJECTS) $(SHLIBS)


%.o: %.c
	$(CC) $(CFLAGS) -o $@ -c $<

%.so: %.o
	$(CC) $(LDFLAGS) $< -o $@

clean:
	rm -f $(OBJECTS) $(SHLIBS)

