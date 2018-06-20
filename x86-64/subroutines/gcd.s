#=============================================================================
# The gcd(a, b) function recursively computes great common divisor of a and b.
# Here, we assume both a and b are non-negative integers.
# a: %edi, b: %esi

.globl gcd

gcd:
        orl     $0, %esi        # Sets zero flag if b = 0
        jnz     gcd_continue    # if b != 0, go to gcd_continue
        movl    %edi, %eax      # otherwise return a
        jmp     gcd_return

gcd_continue:
        pushq   %rdi            # store a into the stack
        pushq   %rsi            # store b
        
        pushq   %rdx            # save %rdx and %rax
        pushq   %rax
        movl    %edi, %eax      # store a into %eax, the dividend
        xorl    %edx, %edx      # clear %edx
        divl    %esi            # perform a / b, with result
                                # quotient in %eax, remainder in %edx
        movl    %esi, %edi      # move b to a: a = b
        movl    %edx, %esi      # move remainder to b: b = a mod b
        popq    %rax            # restore %rdx and %rax
        popq    %rdx
        call    gcd             # make recursive call; answer will be in %eax

gcd_resume:
        popq    %rsi            # restore a and b
        popq    %rdi

gcd_return:
        ret                     # all done!
