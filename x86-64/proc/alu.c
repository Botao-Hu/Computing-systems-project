/*! \file
 *
 * This file contains definitions for an Arithmetic/Logic Unit of an
 * emulated processor.
 */


#include <stdio.h>
#include <stdlib.h>   /* malloc(), free() */
#include <string.h>   /* memset() */

#include "alu.h"
#include "instruction.h"


/*!
 * This function dynamically allocates and initializes the state for a new ALU
 * instance.  If allocation fails, the program is terminated.
 */
ALU * build_alu() {
    /* Try to allocate the ALU struct.  If this fails, report error then exit. */
    ALU *alu = malloc(sizeof(ALU));
    if (!alu) {
        fprintf(stderr, "Out of memory building an ALU!\n");
        exit(11);
    }

    /* Initialize all values in the ALU struct to 0. */
    memset(alu, 0, sizeof(ALU));
    return alu;
}


/*! This function frees the dynamically allocated ALU instance. */
void free_alu(ALU *alu) {
    free(alu);
}


/*!
 * This function implements the logic of the ALU.  It reads the inputs and
 * opcode, then sets the output accordingly.  Note that if the ALU does not
 * recognize the opcode, it should simply produce a zero result.
 */
void alu_eval(ALU *alu) {
    uint32_t A, B, aluop;
    uint32_t result;

    A = pin_read(alu->in1);
    B = pin_read(alu->in2);
    aluop = pin_read(alu->op);

    result = 0;

    /*======================================*/
    /* TODO:  Implement the ALU logic here. */
    /*======================================*/

    switch (aluop) {
    
    // simple addition of A and B
    case ALUOP_ADD:
        result = A + B;
        break;
    
    // bitwise invert of A
    case ALUOP_INV:
        result = ~A;
        break;
    
    // simple substraction of B from A
    case ALUOP_SUB:
        result = A - B;
        break;
    
    // bitwise exclusive-or of A and B
    case ALUOP_XOR:
        result = A^B;
        break;

    // bitwise or of A and B
    case ALUOP_OR:
        result = A|B;
        break;

    // increment A by 1
    case ALUOP_INCR:
        result = A + 1;
        break;

    // bitwise and of A and B
    case ALUOP_AND:
        result = A&B;
        break;

    // arithmetic shift-right (preserve sign)
    case ALUOP_SRA:
        result = A>>1;
        break;
    
    // logical shift-right (add 0 on the left bit)
    case ALUOP_SRL:
        result = (int)((unsigned int)A)>>1;
        break;

    // arithmetic shift-left
    case ALUOP_SLA:
        result = A<<1;
        break;

    // logical shift-left
    case ALUOP_SLL:
        result = A<<1;
        break;
    }

    pin_set(alu->out, result);
}

