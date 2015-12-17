ROOT := $(abspath $(CURDIR)/..)
CC = gcc

COMMON_CFLAGS := -Wall -Werror -Wextra -MMD -MP

TESTS := test-vsnprintf32
TESTS += test-vsnprintf64
TESTS += test-heapsort

.PHONY: test
test: $(TESTS)
	@ set -e; for X in $(TESTS); \
	do \
		echo "Running $$X"; \
		./$$X; \
	done

test-vsnprintf32 : vsnprintf.c
	$(CC) -m32 $(COMMON_CFLAGS) -I $(ROOT)/include -Wno-format -O3 $< -o $@

test-vsnprintf64 : vsnprintf.c
	$(CC) -m64 $(COMMON_CFLAGS) -I $(ROOT)/include -Wno-format -O3 $< -o $@

test-heapsort : heapsort.c
	$(CC) $(COMMON_CFLAGS) -I $(ROOT)/include -O3 $< -o $@

-include $(TESTS:%=%.d)

.PHONY: clean
clean:
	rm -f $(TESTS) *.o
