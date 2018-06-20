# Computing-systems-projects

*These miniprojects are based on homework problems and codes from CS24 Computing Systems, Caltech, Spring 2018.*  
***Honor Code:** If you are taking the course, you must not refer to this repo for solutions.*

## Contents

Low level programming with Assembly language and C, dealing with kernel/OS problems in modern computing systems.  
Projects include:
* x86-64 assembly programming. Subroutine calls, functions, interface with low level C. A run length encoder (RLE) is implemented. See `./x86-64`.

* Heap allocator. An explicit heap allocator is implemented, with best-fit strategy, constant-time deallocation, and explicit free-list. See `./allocator`.

* C implementation of Object-Oriented Programming (OOP, i.e., class, abstraction, inheritance, encapsulation), exceptional flow, and garbage collection. See `./C-specialtopics`.

* CPU cache simulation and optimization (memory monitoring, replacement policies). See `./cache`.

* Thread scheduling library: machine context switch, concurrency issues, mutex, semaphores, thread fairness and safty. See `./thread-scheduler`.

* User-space virtual memory system design, enforced by the processor’s MMU and the OS virtual memory system, and paging policies. See `./virtual-memory`.

## Requirements

* 64-bit version of Linux Mint 18.3 or higher
* gcc, gdb
* make
* valgrind (optional)
