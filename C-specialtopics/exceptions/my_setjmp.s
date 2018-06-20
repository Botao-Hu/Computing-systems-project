.globl my_setjmp

#============================================================================
# my_setjmp:  my implementation of setjmp() function in assembly language
#
# Arguments to my_setjmp are in these registers (following the System V AMD64
# ABI calling convention):
#
#      %rdi = pointer to buffer that is going to hold the current execution
#             state inside the function, refer to it as env_buf.
#
# Return-value in %eax has the default value of zero, but if we jumped back
# using my_longjmp(), to the function who calls my_setjmp() it may seem we
# are returning some other value that is specified inside my_longjmp().
#
my_setjmp:
        # First of all, we need to save callee-save registers into env_buf,
        # including rbp, rbx, r12, r13, r14, r15. Note that each register
        # is of 8 bytes, so every time we store a register, we move the 
        # env_buf pointer by 8 bytes.
        movq    %rbp, (%rdi)
        movq    %rbx, 8(%rdi)
        movq    %r12, 16(%rdi)
        movq    %r13, 24(%rdi)
        movq    %r14, 32(%rdi)
        movq    %r15, 40(%rdi)

        # Secondly, we need to save rsp into env_buf.
        movq    %rsp, 48(%rdi)

        # Thirdly, we store the return address of my_setjmp() function caller
        # so we can come back to the same address using my_longjmp(). Since
        # we didn't change the stack inside the function, rsp is actually
        # pointing to the memory holderf of return address.
        movq    (%rsp), %rcx
        movq    %rcx, 56(%rdi)

        # Now the env_buf has all the information we need if we want to use
        # my_longjmp(). And the size of this buffer is 64 bytes in total. So
        # in the .h file we need the buffer array size to be 8.

        # At last, we return this function with default return value of zero.
        movl    $0, %eax
        ret


.globl my_longjmp

#============================================================================
# my_longjmp:  my implementation of longjmp() function in assembly language
#
# Arguments to my_setjmp are in these registers (following the System V AMD64
# ABI calling convention):
#
#      %rdi = pointer to buffer that is holding the execution state when the
#             my_setjmp() function was called, refer to it as env_buf.
#      %esi = the value that the corresponding my_setjmp() function "should"
#             return after going back from my_longjmp().
#
# The function does not have any return value.
my_longjmp:
        # First of all, restore all the callee-save registers. We just do the
        # reverse of what we did in my_setjmp().
        movq    (%rdi), %rbp
        movq    8(%rdi), %rbx
        movq    16(%rdi), %r12
        movq    24(%rdi), %r13
        movq    32(%rdi), %r14
        movq    40(%rdi), %r15

        # Secondly, restore rsp from env_buf.
        movq    48(%rdi), %rsp

        # Thirdly, we change the return address of this function, which is
        # equivalent to changing the value that rsp is pointing to.
        movq    56(%rdi), %rcx
        movq    %rcx, (%rsp)

        # At last, before return, we store the fake return value into %rax,
        # which is given in %rsi. Note that if we call my_longjmp(env, 0),
        # the corresponding my_setjmp() should return 1 instead of 0.
        test    %esi, %esi                 
        jz      zero_case           # if return value is 0, return 1 instead
        movl    %esi, %eax          # else, return that value
        ret
zero_case:
        movl    $1, %eax
        ret

