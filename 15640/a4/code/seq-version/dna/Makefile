CC=gcc
CFLAGS=-c -Wall -ggdb
LDFLAGS=
HEADERS=strand.h kmeans.h time.h
SOURCES=main.c strand.c kmeans.c time.c
OBJECTS=$(SOURCES:.c=.o)
EXECUTABLE=main

all: $(SOURCES) $(EXECUTABLE)
  
$(EXECUTABLE): $(OBJECTS) 
	$(CC) $(LDFLAGS) $(OBJECTS) -o $@

.c.o:
	$(CC) $(CFLAGS) $< -o $@

clean:
	rm -rf *.o main 
