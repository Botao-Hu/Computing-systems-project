OBJS = c_except.o my_setjmp.o ptr_vector.o divider.o

CFLAGS=-g
ASFLAGS=-g

all: divider check

divider : $(OBJS)
	$(CC) $(LDFLAGS) $^ -o $@

check : test_setjmp.o my_setjmp.o
	$(CC) $(LDFLAGS) $^ -o $@

ptr_vector.c : ptr_vector.h
c_except.c : c_except.h my_setjmp.h
divider.c : c_except.h my_setjmp.h
test_setjmp.c : my_setjmp.h

clean :
	rm -f *.o *~ divider check

.PHONY: all clean
