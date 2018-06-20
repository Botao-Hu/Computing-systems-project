#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_setjmp.h"


static jmp_buf env;


int sub_test1(int return_value);
int test1();


int sub_test2_f(int x);
int sub_test2_g(int x);
int sub_test2_h(int x);
int test2();


int sub_test3(int type);
int test3();


/* TEST 1: RETURN VALUE VERIFICATION
 * This is the basic test to tell whether the longjmp() can change the
 * return value of setjmp() correctly. In order to get the return value,
 * this test is done inside one function.
 */
int sub_test1(int return_value) {
    int temp = setjmp(env);
    if (temp != 0) {
        /* after longjmp() */
        printf("longjmp(env, %d) returns %d: ", return_value, temp);
        /* longjmp(env, 0) would force setjmp(env) to return 1 instead */
        if (return_value == 0) {
            if (temp == 1) {
                printf("PASS\n");
            }
            else {
                printf("FAILED\n");
            }
            return (temp == 1);
        }
        /* otherwise, return that value */
        else {
            if (temp == return_value) {
                printf("PASS\n");
            }
            else {
                printf("FAILED\n");
            }
            return (temp == return_value);
        }
    }
    longjmp(env, return_value);
}


/* This function does test 1 on various return values. */
int test1() {
    printf("Doing test 1: return value verification...\n");
    int i = 0;
    int flag = 1;
    while (i < 10) {
        if (!sub_test1(i)) {
            flag = 0;
        }
        i++;    
    }
    if (flag) {
        printf("Test 1 PASS, congrats!\n");
        return 1;
    }
    else {
        printf("Test 1 FAILED.\n");
        return 0;
    }
}


/* TEST 2: JUMP ACROSS MULTIPLE FUNCTIONS
 * This test aims at verifying if longjmp() can correctly jump across multiple
 * function invocations. The following three functions are from the lecture, 
 * and are good examples to see how the longjmp() acts. We simplify this
 * procedure to make life easier.
 */
int sub_test2_f(int x) {
    if (setjmp(env) == 0) {
        return sub_test2_g(x);
    }
    else {
        return -1;
    }
}

int sub_test2_g(int x) {
    return sub_test2_h(x);
}

int sub_test2_h(int x) {
    if (x < 5) {
        longjmp(env, 1);
    }
    return -100;
}


/* This function runs test 2 for many times with different inputs. For input
 * less than 5, the longjmp() would force the return value to be -1, otherwise
 * return -100. This function also prints out each result for debugging.
 */
int test2() {
    printf("Doing test 2: jump across multiple functions...\n");
    int i = 0;
    int flag = 1;
    int temp = 0;
    while (i < 10) {
        temp = sub_test2_f(i);
        printf("h(g(f(%d))) returns %d: ", i, temp);
        if ((i < 5 && temp == -1) || (i >= 5 && temp == -100)) {
            printf("PASS\n");
        }
        else {
            printf("FAIL\n");
            flag = 0;
        }
        i++;
    }
    if (flag) {
        printf("Test 2 PASS, congrats!\n");
        return 1;
    }
    else {
        printf("Test 2 FAILED.\n");
        return 0;
    }
}


/* TEST 3: STACK CORRUPTION AVOIDANCE
 * This test aims at verifying the jump functions does not corrupt the stack,
 * i.e., changing local variables, messing up %rsp and callee-save registers.
 * To do this, we put known variables before setjmp() and after setjmp() and
 * see if they are not changed; also we put variables after longjmp() is
 * called, to see if longjmp() correctly restores these values.
 */
int sub_test3(int type) {
    
    /* put some known variables before setjmp() */
    int case_1 = 75968;
    double case_2[4] = {1000.0, 2.1, 3.4, 50.9};
    char case_3[] = "HelloWorld";
    int flag = 1;

    if (setjmp(env) == 0) {
        if (type) {
            /* After setjmp() is called, see if these variables change */
            printf("After setjmp():\n");

            int case_1_1 = 75968;
            double case_2_1[4] = {1000.0, 2.1, 3.4, 50.9};
            char case_3_1[] = "HelloWorld";

            printf("Integer = %d, should be %d: ", case_1, case_1_1);
            if (case_1_1 == case_1) {
                printf("PASS\n");
            }
            else {
                flag = 0;
                printf("FAIL\n");
            }
            
            printf("Double array = [%.1f %.1f %.1f %.1f], ", 
                        case_2[0], case_2[1], case_2[2], case_2[3]);
            printf("should be [%.1f %.1f %.1f %.1f]: ",
                        case_2_1[0], case_2_1[1], case_2_1[2], case_2_1[3]);
            if (case_2_1[0] == case_2[0] && case_2_1[1] == case_2[1] &&
                case_2_1[2] == case_2[2] && case_2_1[3] == case_2[3]) {
                printf("PASS\n");
            }
            else {
                flag = 0;
                printf("FAIL\n");
            }

            printf("String = \"%s\", should be \"%s\": ", case_3, case_3_1);
            if (!strcmp(case_3, case_3_1)) {
                printf("PASS\n");
            }
            else {
                flag = 0;
                printf("FAIL\n");
            }

            return flag;
        }        
    }
    else {
        /* After longjmp() is called, see if these variables change */
        printf("After longjmp():\n");

        int case_1_2 = 75968;
        double case_2_2[4] = {1000.0, 2.1, 3.4, 50.9};
        char case_3_2[] = "HelloWorld";

        printf("Integer = %d, should be %d: ", case_1, case_1_2);
        if (case_1_2 == case_1) {
            printf("PASS\n");
        }
        else {
            flag = 0;
            printf("FAIL\n");
        }
        
        printf("Double array = [%.1f %.1f %.1f %.1f], ", 
                    case_2[0], case_2[1], case_2[2], case_2[3]);
        printf("should be [%.1f %.1f %.1f %.1f]: ",
                    case_2_2[0], case_2_2[1], case_2_2[2], case_2_2[3]);
        if (case_2_2[0] == case_2[0] && case_2_2[1] == case_2[1] &&
            case_2_2[2] == case_2[2] && case_2_2[3] == case_2[3]) {
            printf("PASS\n");
        }
        else {
            flag = 0;
            printf("FAIL\n");
        }

        printf("String = \"%s\", should be \"%s\": ", case_3, case_3_2);
        if (!strcmp(case_3, case_3_2)) {
            printf("PASS\n");
        }
        else {
            flag = 0;
            printf("FAIL\n");
        }
        return flag;
    }
    longjmp(env, 1);
}


/* This function runs test 3 after setjmp() and after longjmp(), and print
 * the overall result.
 */
int test3() {
    printf("Doing test 3: stack corruption avoidance...\n");
    int f1 = sub_test3(1);
    int f2 = sub_test3(0);
    if (f1 && f2) {
        printf("Test 3 PASS, congrats!\n");
        return 1;
    }
    else {
        printf("Test 3 FAILED.\n");
        return 0;
    }
}


/* TEST 4: ???
 * This test aims at... 
 */

int main() {
    printf("Setjmp Test Program Start!\n");
    /* run test 1 */
    int f1 = test1();
    /* run test 2 */
    int f2 = test2();
    /* run test 3 */
    int f3 = test3();
    printf("=================================\n");
    if (f1 && f2 && f3) {
        printf("All tests PASSed. Congratulations!\n");  
    }
    else {
        printf("Some of the tests FAILed. Go and check!\n");
    }
}
