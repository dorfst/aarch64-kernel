# What is this project?
This project is a small kernel that sets up virtual memory and runs
simple processes that use cooperative scheduling, displaying kernel status
by printing to PL011 UART.

This project does get updated from time-to-time, so what you see is not necessarily the final product.

AArch64, bare-metal, virtual memory (MMU), MMIO, exception handling, linker scripts, process scheduling, cross-compilation toolchains

# If you're short on time
This kernel implements 32-bit virtual memory, cooperative process scheduling
and exception handling. The best things to check out would be
- [`save_process.s`](src/save_process.s) and its [debugging story](DEBUGGING.md#save_process)
- [the memory section](#memory)
- [the scheduling section](#scheduling)

# How do I run this project?

There are a few dependencies to sort out:
- `aarch64-linux-gnu-gcc`
- `aarch64-linux-gnu-ld`
- `aarch64-linux-gnu-as`
- `gdb-multiarch` (debian-based) or `gdb` if you're already on AArch64 or your `gdb` build supports multiple architectures
- `qemu`, specifically having `qemu-system-aarch64`
- `make`

There are flags in the Makefile already to make sure that the linux compiler ends up compiling for
a freestanding environment.

The recommended way is to run `qemu-quickstart.sh`:
1. run `./qemu-quickstart.sh`
2. press `Ctrl+A+x` (that is, `Ctrl+A`, then `x`) to exit QEMU when the kernel halts.

This is the expected output:
```
memory copy success!!!yielding process 0x0000000000000001
loading process 0x0000000000000002
yielding process 0x0000000000000002
loading process 0x0000000000000001
yielding process 0x0000000000000001
loading process 0x0000000000000002
yielding process 0x0000000000000002
loading process 0x0000000000000001
quitting process 0x0000000000000001
freeing memory
loading process 0x0000000000000002
yielding process 0x0000000000000002
loading process 0x0000000000000002
yielding process 0x0000000000000002
loading process 0x0000000000000002
yielding process 0x0000000000000002
loading process 0x0000000000000002
quitting process 0x0000000000000002
freeing memory
kernel done, halting now
```

If you want to run in debug mode, here's a list of steps of how to run this project. You will need two terminal windows/tabs:
1. In one, run `./qemu-debug.sh`
2. In the second, run `gdb-multiarch obj/kernel.elf` or `gdb obj/kernel.elf` depending on your system.
3. In the `gdb` window/tab, type in `target remote localhost:1234`
4. (Optional) type in `break main.c:74` to set a breakpoint nearest the actual kernel's scheduling of processes, then type `continue`
5. (Optional) type in `display /ni $pc`. This gives you the next `n` instructions to be executed every time you `s/n` or `si/ni`
6. Whenever you're done, exit with `Ctrl+A+x`

Slightly complicated, but due to the nature of the kernel, and the fact that you can see the
kernel executing user processes then switching back to the kernel to context switch, that's how
I'd recommend to run it.

If you just want it to run, you can always enter the debugger and type in `continue`, and look at the output.

# Motivation
Much like the bytecode VM project, I wanted to do something different, difficult, something that
I had never done before, except this was motivated by a university module that went over a few operating system
concepts, but did so in seemingly small detail. After doing this project, a _lot_ was evidently left out
but given what I've experienced and the general goals of a university module
it is definitely understandable.

It was also something that I have thought about doing for a long time generally, but only now
felt ready to tackle it.

As a side note, through reading through this README and reading the source code you will find that
I have made some suboptimal decisions regarding the design of the kernel. They are intentional and serve to make it
easier for me to learn what I wanted to learn from this project (setting up virtual memory and scheduling processes).



# Overview
This project is deceptively simple. Upon booting the kernel sets up support for virtual memory, and then
loads some dummy processes (with the kernel setting up the parameters necessary), which are just `for` loops
written in AArch64 assembly. The vast majority of work done in this project was essentially
setting up the infrastructure required to run even the simplest processes.

That explains why the README is relatively sparse compared to something like my  bytecode VM project.
For the kernel project much of the work is trying to merge two worlds together: how I want my kernel to interface memory allocation
and process scheduling, and making that actually conform to the CPU architecture. It's fundamentally a difference between difficulty
in conforming to already-defined and very strict correctness vs. designing something that is correct and suitable.

However, it is worth bringing up that despite the final product's simplicity, this was no easy feat at all. Lots of work got into setting up the MMU correctly,
setting up page tables correctly, context switching correctly, and making sure state is saved correctly across multiple contexts and between
two exception levels.

In order to understand the work that went into this, I recommend that you check out `DEBUGGING.md`
to get a clearer picture of how difficult a project like this can be.

# Current Limitations
- very unsafe memory usage. While functions are made to issue null pointers in the event of some problem, they are never actually checked
- fixed-size memory allocation
- assumes that a program can fit in one page
- generally speaking a lot of hardcoded sections
- assumes that you're using my [`dummy.s`](src/dummy.s) program, and all of the stuff that goes into loading two instances of that program is hardcoded.
- excess memory usage whenever a system call happens. A frame is allocated for the handler but the syscall handlers call to load a process which never returns back, and instead goes down an exception level with `eret`
- exception handler assumes exceptions happen only for system calls, so errors are not caught properly, just a kernel hang
- expects to run with exactly 128 MiB of RAM
- inefficient TLB management

This is not necessarily an exhaustive list, as there may have been things I've missed or only mentioned in the main document body.


# Memory
## Layout
The memory layout is physically that the lower half of RAM belongs to the kernel
and the upper half of RAM belongs to user space. It was mostly a decision to make sure
that I'm correctly handling kernel and user-space matters separately when debugging, and besides that
doesn't actually hold any benefit and is actually quite wasteful of memory.

If the size of RAM is 128M (as I have been using personally), then the kernel owns 64 MiB of the lower **physical** addresses, and
user space owns 64 MiB of the higher **physical** addresses (although for user-space it is better to
say that that space is used rather than "owned").

The virtual address space is a 32-bit address space, both for kernel and user space. This is because
for the kernel it needs to map at the very least 1 GiB worth of addresses for the QEMU `virt` platform, as
RAM starts at `0x4000_0000`, and everything under that is reserved + MMIO. The physical-to-virtual address mapping
used in this kernel is a direct map (i.e. physical address + fixed offset).

## Tracking memory usage
The kernel uses a memory map of all the addressable space, i.e. MMIO + reserved regions + RAM, and
uses `enum`s to denote usage such as `KERNEL_FREE`, `KERNEL_USED`, `USER_FREE`, `USER_USED`, etc. The memory
map cuts memory into 4KiB chunks (page size).

Upon booting the kernel maps all of memory rather than lazily updating upon an allocation to make memory
allocation mechanistically simpler.

Allocations update this memory map which the kernel has access to at all times.

## Allocating/freeing memory
Allocating memory is as simple as updating the memory map, and returning the pointer of the next available 4 KiB chunk. To make it clear,
memory allocation is quite simple in that one allocation allocates specifically 4 KiB to one process, without any option
for more granular allocation. This is deliberate as the aim of this project was not exactly to go _extremely_ deep on memory management,
which is a rabbit hole in and of itself.

Upon both allocation and freeing the page is zeroed (i.e. all bytes are set to zero).

In the current state of freeing memory, this only frees a process's program page. The page tables are not freed as of now.


## Virtual memory
Virtual memory on the AArch64 architecture works by using multi-level page tables (generally speaking L0-L3, although more levels do exist), with
varying granule (i.e. page) sizes available. Each table has 64-bit descriptors which point to other tables, describe a block of memory, or describe a page, depending
on the level of the table. Check out the Arm Architecture Reference Manual's VMSA chapter to see how virtual addresses map to
physical addresses and how the page table entries work specifically.

This kernel implements 32-bit virtual addressing, with one set of tables for the kernel, and tables for processes being generated as they are loaded.
I chose 32-bit virtual addressing for kernel space because I need to map at least 1 GiB of addressable space (in total 1GiB + 128MiB)
as something the kernel can access. User processes get 32-bit addresses out of consistency rather than any actual constraint.

I also decided to implement virtual memory since it is a core feature of many operating systems and was interested in the mechanism behind it.
Getting something to the stage of working is relatively simple yet still very difficult. Virtual memory is also its own
rabbit hole when it comes to things like ASIDs and proper management of the TLB.



# Processes
## Process control blocks
Process control blocks contain essential information like process ID and process context.
If you read [`proc.h`](src/proc.h) you may notice that there is an `exec_time` field that never gets used. My original idea
for the kernel was to use preemptive scheduling based on timer interrupts so I could approximately calculate the time
for which a process has been running, but due to time constraints I settled
on a simpler cooperative scheduling approach.

As an extra note, you may notice that processes have a process state, but in this kernel they're actually
not really used for anything. They're there because it's a standard thing to have and I wasn't sure if I was
going to need them or not.

## Scheduling
There is a process queue (10 processes max) from which the scheduler switches processes.
The scheduling algorithm is cooperative round-robin scheduling. A process yields control by making
a system call, which then causes the scheduler to context switch and continue execution.

A process can also make a system call to tell the kernel that it's done executing, at which point the kernel will clean up
the allocated memory and remove it from the process queue.

## Context switching
Context switching is the mechanism of storing a process's state in memory and
loading another process's state into the CPU from memory.
The context switch saves:
- all 31 general-purpose registers
- the EL0 stack pointer
- the program counter (technically `ELR_EL1` but functionally the same)
- the process state (`SPSR_EL1`)

which, plus the address of the highest level page table, is the process context saved in the process control blocks.

Upon loading _in_ a process's context the TLB is also fully invalidated to make sure that the TLB has correct entries cached.
It's not particularly efficient to flush the whole TLB but given that I'm not using ASIDs that's the simplest
thing to do, and efficient TLB management was out of scope for the project while I was trying to get the basics
of memory management and process scheduling to work.

# Exception handling & system calls

Exception handling is an important part of this project as it is what allows for context switching to happen in the first place.

AArch64 expects an exception vector that contains 16 entries, where each entry contains at _most_ 32 instructions
to handle exceptions. Which entry gets chosen depends on exception type, architecture (32 vs. 64-bit) and change in exception level.

At the moment the exception vector is essentially filled with dummy entries, except at offset 0x400, which is for
synchronous exceptions on AArch64 that cause a rise in exception level.

The convention for making system calls in my kernel is:
```asm
mov x8, #imm
svc #0
```
This is identical to the way that Linux expects system calls on AArch64. For my purposes specifically,
it wouldn't make a difference whether I put the system call number in a register or as the immediate argument
in `svc`.

Currently I have two syscalls:
- `yield` - system call number `20`. Hand control of the CPU back to the kernel for a context switch.
- `exit` - system call number `21`. Call the kernel to clean up the process as it has finished executing.

The choice for the system call numbers is arbitrary, just as something that would
plausibly have to be deliberately chosen.


# I/O
The only I/O involved in this project is the use of a PL011 UART on the `virt` machine to display
kernel status to the terminal.

There isn't much to be said about this since getting the PL011 UART to work is a very mechanical and fixed procedure
that essentially has no design decision made by me behind it. Additionally, it was not exactly part of my learning
objectives but I did appreciate actually experiencing what the interface between code and MMIO devices
is like.

There is no file system or support for disks in this project, and is deliberately done that way as, like many things
in operating systems, is its own rabbit hole, and I wanted to learn about the absolute core upon which
everything else relies.

# Linking
I thought this deserved its own section because, while it is not directly related to the kernel, it is an essential
part of it that I had absolutely no idea I'd need and rely on so much going in to this project.

## VMAs and LMAs

Having to implement virtual memory made linking a more important aspect of the project as opposed to something
that I just have to do because I'm not running on any operating system.

In AArch64 32-bit virtual addressing,
the most significant 32 bits are all ones if you plan on the translation provided by the table pointed to by the `TTBR1`
register. This is typically (but not necessarily) the one used for the kernel's translations since it makes it
a bit simpler for linking user programs where they conventionally expect low addresses to be available.

Using linker scripts, you first have to specify what address you wish to use to link a particular section (the VMA), but
you can specify where you want it to physically be loaded (the LMA).

### Boot stage
Dealing with virtual and physical addresses does get a little complicated when booting up because
you have to deal with the situation where you turn the MMU on, but you have not yet transitioned to the code
properly linked to high virtual addresses. For AArch64 specifically, the typical solution is to
map the boot code with one set of page tables pointed to by `TTBR0` as an identity map, and map the rest to another set of page
tables pointed to by `TTBR1`. After that, you can point user processes to `TTBR0` as the boot process has been completed.

## Custom sections & linker symbols
In this project, having the ability to make custom sections is a godsend because it lets me cleanly separate code such as boot code and exception vector code
either because it requires different linking or for the sake of organisation. 

Linker symbols are great because I can know of important regions of memory quite easily, such as where exactly my exception vector
begins, where the end of the kernel code is (to start putting data structures there), etc.
It's really convenient.

# Debugging
In my opinion, this is the more interesting part about this project, especially since I probably spent more time debugging
than actually thinking about how to design the kernel. Check out [`DEBUGGING.md`](DEBUGGING.md) for some
highlights from my debugging journey.

# Conclusion
This was a very rewarding, yet difficult project to do. It really put me out of my comfort zone,
taught me a lot about how programs get made when you need to specify _how_ your code gets linked, got
me acquainted with using cross-architecture toolchains and was a good experience of programming in an environment
where you are expected to know _exactly_ what you are doing rather than a supporting operating system
nicely handling your mistakes.

That being said, had I more time on this project, I would do a variety of things different:
- more elaborate memory allocation with support for smaller allocation sizes
- much cleaner separation of subsystems rather than a global kernel state
- generalised support for processes rather than tailoring the process creation to my dummy processes
- better I/O support for things like disks and file systems
- a cleaner way of dealing with pointer arithmetic, especially since I had to bypass C's conventional model of it (check the debugging section for more info)

At that point, though, I'd essentially be making my own operating system, which would take this project
_way out_ of scope.




