# What is this project?
This project is a toy kernel that implements basic features such as memory allocation, virtual memory, process scheduling and basic interaction with an I/O device.

# Overview
This kernel is made for AArch64, Arm's 64-bit CPU architecture, specifically made for the QEMU `virt` virtual machine. It runs a bunch of dummy processes that do pretty much nothing and only exist to showcase the scheduler and especially context switches.

# I/O
The way that I've chosen to see the kernel's status is to print messages out to PL011 UART.

# Virtual Memory
