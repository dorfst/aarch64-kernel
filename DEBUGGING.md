# Debugging
This section is about the problems I had while making this project. Believe me, there
were many. This is quite an interesting thing to discuss because it really shows the process
of making a project like this. Comparing the final result with the work done provides an unusual
contrast compared to more conventional software development routes in modern times.

# Pointer Arithmetic
Welcome to the land of type casting, where you can be whoever you want to be. This isn't one specific bug, but a category of bugs that bit me
in the behind more than I would like to admit, and it comes down to a key feature of how C does pointer arithmetic.

The way that C does pointer like this: 
- imagine that you have a pointer `ptr` of the type `uint64_t*`,
- you would like to do `ptr + 1`
- the result in reality is `(uint64_t)ptr + 8`.

To put it simply, an increment in a pointer scales to `sizeof(type)`. In the example, a `uint64_t` is 8 bytes
large, so `ptr + 1` will actually add 8 to `ptr` if you were to treat it as a `uint64_t`. This is very useful
in the common case of arrays.

For example, when I'm building page tables for a process I have to `malloc` space
for the table, which returns a pointer (as generally speaking that's the point of `malloc`), but I do
have to immediately cast it to a `uint64_t` so that my arithmetic is applied exactly as written.

The symptom in many cases would be a synchronous exception of some sort. As of writing this, the specific
instances of this issue have been blurred together, but I do remember it being particularly an issue in the
`mark_used_kernel_areas` function in [`boot.c`](src/boot.c) where I'm using linker symbols (which are typed as pointers), but I need
to use them like `uint64_t`s. I don't remember the symptom in this specific case, unfortunately.

You have to be very careful about how you write your casts, otherwise you will get
values that make no sense.

It's also just annoying because it makes the code messy for some arithmetic to have `(uint64_t)ptr`s whenever
you just want to use a pointer like a regular number.

I don't think `typedef`s would change this because you're just doing the same cast with a prettier name,
like `uintptr_t`.

# `save_process`
This one is regarding context switching, particularly when I save process context.

When you save a process's context, you save that content in memory in the process's designated
PCB. What a beautiful and simple concept! What could go wrong?

Quite a bit, actually, to a very amusing degree.

The symptom was this: I'd load in a process for its first round of execution. Fine. I'd execute that process. Fine. I'd save the process
context. As of now, fine. Load in another process for its first round of execution and continue as stated just now. As of now, fine.

Now load the process in for its second round of execution. Strange... only two instructions are valid?
Why is the rest of the program undefined?

There are two components to context switching: saving the process context in memory, and saving it in the _correct_ place in memory.

Saving it in memory is the very easy part. Saving it in the correct place is also easy, but I got it wrong anyway.

So, the `save_process` function had previously expected that the register `x0` contained the address
where the process context in the PCB begins. From the dummy program and the way that I initialise processor state, `x0` is zero,
and stays that way since the dummy program never uses `x0`.

Now, the [`save_process`](src/save_process.s) function saves the general-purpose registers, the program counter (though technically `ELR_EL1`) and the process state `SPSR_EL1`.
In the [`proc_context`](src/proc.h) struct that goes from the second to the last entry. The first entry
is the L1 page table address which is a `uint64_t`, so 8 bytes in size. Everything [`save_process`](src/save_process.s) saves
is from 8 bytes into the struct onward. This is very important to how I figured this bug out.

Remember how I said only two instructions were valid? To be specific, those were the first two instructions that were valid.
AArch64 instructions are 4 bytes, so only the _first_ 8 bytes of the program are preserved, and my `proc_struct` only saves stuff
from the 8-byte offset and beyond. What a coincidence!

And remember how I said that `x0`'s value is zero? My programs are loaded at virtual address `0x0` (not good practice, but it is simple).
Things are starting to piece together.

To put it short, I was overwriting the program's instructions with its own processor state, rather than in its own PCB.
Fixing this took a bit of assembly magic where I save the value of `x0` on the kernel stack and then load it with the PCB. Specifically,
I use the first process struct in the process queue, and then load `x0` with the memory address where the [`proc_context`](src/proc.h) stored inside the
`proc` struct is.

I call it assembly magic because apparently you can `adrp` C variables and then add their offset into the page. That's quite cool, and I didn't
know that was possible.

# Ternary Operator Woes
This bug is an interesting one because the fix is so simple, but it was not a fun one to figure out.

When setting up the memory map, I set some of the pages to free, kernel reserved `KERNEL_FREE` and free,
user reserved `USER_FREE`.

So, what that means, is that whenever I'd allocate memory it wouldn't find any free page, and then
return a null pointer (which, by my own mistake, I didn't check despite designing the allocator function that way).

What happens from that point is that I end up writing page table entries to the null address without error.
I load the necessary values into the registers, most importantly with `TTBR0/1` being loaded
with null addresses. Once the MMU is turned on this leads to an immediate translation fault.

This sounds like quite a conundrum. However, the mistake, while critical, is not so convoluted.

I had structured the ternary operator to decide whether a page is `KERNEL_FREE` or `USER_FREE`
via the syntax `KERNEL_FREE ? current_address < kernel_space_boundary : USER_FREE`, instead of
`current_address < kernel_space_boundary ? KERNEL_FREE : USER_FREE` (you won't find this exact expression
in [`boot.c`](src/boot.c))

`KERNEL_FREE`'s value as an enum is 2. When evaluated as a truthy value this is always true.
So, what this means via the correct C syntax is that the value of the enum is going to be determined by the condition,
i.e. `current_address < kernel_space_boundary`. It's true for the kernel space, so it gets the value 1, which
is the `PAGE_T` struct. Otherwise, in the reserved user space, that condition is 0, so it resolves to `PAGE_S`.

What I like about this bug is how simple, yet catastrophic, it is. A simple syntax error that is technically still
valid creates a chain of events that leads to the totally wrong and unintended outcome.

# Physical-to-virtual address switch
This one is relatively simple, but I like this one just because I had missed something vital to the 
switch between an MMU-disabled and MMU-enabled environment.

I had been getting a data abort, translation level 0 fault, and was inspecting the register state in GDB.
I just happened to realise that the stack pointer had not been set properly. It was pointing to a physical address
rather than a high canonical virtual address.

This meant two things: the first being simply that the stack pointer is not at a high VA so whenever you'd load
in an actual user process you'd get rid of the kernel stack, but secondly anything in that stack now is stored at
essentially stale addresses.

Thankfully, nothing in the stack at that point would be needed any further by the kernel, so you can just make a whole new
stack at a high VA, set `SP_EL1` to that new stack and go into the main part of the kernel.

This one is interesting because when exploring unchartered territories it's quite easy to miss out things
if you're not careful enough.


# Permission Bits
This one is a good lesson in double-checking rather than assuming something is correct.

When making page table entries for user processes, you must set the fields so that it allows unprivileged access.
It sounds simple enough, and it is, but I had forgotten that what I was copying did not have this
crucial attribute in the descriptor.

The problem is that for kernel memory the permission bits only allow for privileged memory access.
This is what some would call good design. It's certainly not good design to put that for memory that can be
accessed in EL0 so that you get permission faults at level 3.

The solution is obvious: make sure that table descriptors and page descriptors allow for unprivileged execution and access.
Specifically, the `UXNTable` bit for table descriptors and the `AP` bits for page descriptors.

Don't assume that because it works here that it will work there, especially when it concerns configuration.

