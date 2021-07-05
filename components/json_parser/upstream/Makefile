CC := gcc
CFLAGS := -O2 -Iinclude -I.

all: json_parser

json_parser: src/json_parser.c tests/main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

clean:
	@rm -f *.o json_parser
