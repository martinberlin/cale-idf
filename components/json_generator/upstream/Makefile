CC := gcc
CFLAGS := -O2 -I.

all: json_gen

json_gen: test.o json_generator.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	@rm -f *.o json_gen
