#include <stdio.h>
#include <stdlib.h>
#include "fact.h"

int main(int argc, char** argv) {
    // if there is not one command-line argument, report error
    if (argc != 2) {
        printf("Error: we only accept one command-line argument\n");
        printf("Usage: ./factmain non_negative_integer_value\n");
        exit(1);
    }

    // parse the argument into an int variable
    // here, we assume the command-line argument is an integer
    int n = atoi(argv[1]);

    // if the number is negative, report error
    if (n < 0) {    
        printf("Error: we only accept non-negative integers\n");
        exit(1);
    }

    // run fact(n) in assembly code and print the result
    int result = fact(n);
    printf("%d\n", result);
}
