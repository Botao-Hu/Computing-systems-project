CC = gcc
CFLAGS = -Wall -Werror -g -O0

VMEM_OBJS = virtualmem.o vmalloc.o matrix.o test_matrix.o

# So that the binary programs can be listed in fewer places.
# You will want to add to this variable as you implement various policies.
BINARIES = test_matrix test_matrix_clock test_matrix_age


all: $(BINARIES)

# Compile this file with optimizations so that it accesses memory in
# reasonable ways.
matrix.o: CFLAGS += -O2

test_matrix: $(VMEM_OBJS) vmpolicy_random.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_matrix_clock: $(VMEM_OBJS) vmpolicy_clock.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

test_matrix_age: $(VMEM_OBJS) vmpolicy_age.o
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDFLAGS)

clean:
	rm -f *.o *~ $(BINARIES)


.PHONY: all clean

