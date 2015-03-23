ROOT := $(abspath $(CURDIR)/..)

COMMON_CFLAGS := -Wall -Werror -Wextra -MMD -MP

TESTS := test-vsnprintf32
TESTS += test-vsnprintf64

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

-include $(TESTS:%=%.d)

.PHONY: clean
clean:
	rm -f $(TESTS) *.o
