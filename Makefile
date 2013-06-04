SOURCES=ramparse.c packet.c

OBJECTS=$(SOURCES:.c=.o)
HEADERS=$(wildcard *.h)
EXEC=ramparse
MY_CFLAGS += -Wall -O0 -g -std=c99 -pedantic -Werror
MY_LIBS += -lpthread -lrt

all: $(OBJECTS)
	$(CC) $(LIBS) $(LDFLAGS) $(OBJECTS) $(MY_LIBS) -o $(EXEC)

clean:
	rm -f $(EXEC) $(OBJECTS)

%.o: %.c ${HEADERS}
	$(CC) -c $(CFLAGS) $(MY_CFLAGS) $< -o $@

