CC      ?= gcc
CFLAGS  ?= -std=c11 -Wall -Wextra -Werror -g -fsanitize=address,undefined
INCLUDE  = -I../components/gift_core

test: test_gift
	./test_gift

test_gift: test_gift.c ../components/gift_core/gift_core.c ../components/gift_core/gift_core.h
	$(CC) $(CFLAGS) $(INCLUDE) test_gift.c ../components/gift_core/gift_core.c -o $@

clean:
	rm -f test_gift
