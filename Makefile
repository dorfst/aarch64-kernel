CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld
AS = aarch64-linux-gnu-as

CFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -ffreestanding -std=gnu99 \
         -mcpu=cortex-a72 -mgeneral-regs-only -fno-stack-protector -fno-pic -fno-pie \
         -fno-builtin -fno-asynchronous-unwind-tables -fno-unwind-tables -g

LDFLAGS = -T ./src/link.ld -nostdlib -static

EXCLUDE ?=

SRCDIR = src
OBJDIR = obj

SRCS = $(filter-out $(EXCLUDE), $(wildcard $(SRCDIR)/*.c))
ASMSRCS= $(filter-out $(EXCLUDE), $(wildcard $(SRCDIR)/*.s))

OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
ASMOBJS = $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(ASMSRCS))

OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS)) $(patsubst $(SRCDIR)/%.s,$(OBJDIR)/%.o,$(ASMSRCS))
TARGET = $(OBJDIR)/kernel.elf

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR)/%.o: $(SRCDIR)/%.s
	mkdir -p $(OBJDIR)
	$(AS) -c $< -o $@

$(TARGET):$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

run: $(TARGET)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 128M -nographic -serial mon:stdio -kernel $(TARGET)

debug: $(TARGET)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 128M -nographic -serial mon:stdio -kernel $(TARGET) -s -S

clean:
	rm -rf $(OBJDIR)

.PHONY: all run debug clean