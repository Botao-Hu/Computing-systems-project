/* This file contains x86-64 assembly-language implementations of three
 * basic, very common math operations.
 * COMMENT: the common theme of these three implementation is that
 * they avoid conditional transfer of control, where the program has
 * to follow one execution path when a condition holds and another path
 * when the condition does not hold. Instead, these implementation used
 * conditional transfer of data, where they computes all outcomes of a
 * conditional operation and then selects one based on whether the 
 * corresponding condition hold or not. The advantage of conditional
 * transfer of data is that they can be implemented by a simple
 * conditional move instruction. In this way, we can get faster
 * operation speed (since the instruction is better matched with the
 * characteristic of modern processors), and we can avoid recording
 * multiple execution paths so that we simplify the instruction and 
 * lower the chance of getting errors.
 */

        .text

/*====================================================================
 * int f1(int x, int y)
 * OPERATION: This function is max(x, y)
 */
.globl f1
f1:
        movl    %edi, %edx // %edx = x
        movl    %esi, %eax // %eax = y
        cmpl    %edx, %eax 
        cmovg   %edx, %eax // if x > y, %eax = x; else %eax = y
        ret


/*====================================================================
 * int f2(int x)
 * OPERATION: This function is abs(x)
 */
.globl f2
f2:
        movl    %edi, %eax // %eax = x
        movl    %eax, %edx // %edx = x
        sarl    $31, %edx // if x < 0, %edx = -1; else %edx = 0
        xorl    %edx, %eax 
        // if x < 0, %edx = -1 = 0xFFFFFFFF, XOR flips all bits
        // in x, then since x < 0, %eax = - x - 1;
        // else if x >= 0, %edx = 0, XOR does not change x in %eax, 
        // so %eax = x.
        subl    %edx, %eax 
        // before subl, if x < 0, %eax = -x - 1, and %edx = -1
        // so after subl %eax = -x
        // else if x >= 0, %eax = x, %edx = 0 before subl,
        // so after subl %eax = x
        ret


/*====================================================================
 * int f3(int x)
 * OPERATION: This function is sign(x)
 */
.globl f3
f3:
        movl    %edi, %edx // %edx = x
        movl    %edx, %eax // %eax = x
        sarl    $31, %eax // if x < 0, %eax = -1; else %eax = 0
        testl   %edx, %edx 
        // testl sets OF = 0 and CF = 0
        // if x < 0, SF = 1; else SF = 0
        // if x = 0, ZF = 1; else ZF = 0
        movl    $1, %edx // %edx = 0x1
        cmovg   %edx, %eax 
        // if x = 0, then SF = 0, OF = 0, ZF = 1, 
        // move condition = 0, %eax remain its value, %eax = 0
        // if x < 0, then SF = 1, OF = 0, ZF = 0,
        // move condition = 0, %eax = -1
        // if x > 0, then SF = 0, OF = 0, ZF = 0
        // move condition = 1, %eax = 1
        ret

