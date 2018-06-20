#include <stdio.h>
#include <stdlib.h>
#include "gcd.h"

int main(int argc, char** argv) {
    // if there is not two command-line arguments, report error
    if (argc != 3) {
        printf("Error: we only accept two command-line argument\n");
        printf("Usage: ./gcdmain non_negative_integer_value1 value2\n");
        exit(1);
    }

    // parse the argument into an int variable
    // here, we assume the command-line argument is an integer
    int a = atoi(argv[1]);
    int b = atoi(argv[2]);

    // if the number is negative, report error
    if (a < 0 || b < 0) {    
        printf("Error: we only accept non-negative integers\n");
        exit(1);
    }

    // run gcd(a, b) in assembly code and print the result
    int result = gcd(a, b);
    printf("%d\n", result);
}
