CC = aarch64-linux-gnu-gcc
LD = aarch64-linux-gnu-ld

CFLAGS = -Wall -Wextra -Werror -nostdlib -nostartfiles -ffreestanding -std=gnu99 \
         -mcpu=cortex-a72 -mgeneral-regs-only -fno-stack-protector -fno-pic -fno-pie \
         -fno-builtin -g

LDFLAGS = -T linker.ld -nostdlib -static

SRCDIR = src
OBJDIR = obj

SRCS = $(wildcard $(SRCDIR)/%.c)
OBJS = $(patsubst $(SRCDIR)/%.c,$(OBJDIR)/%.o,$(SRCS))
TARGET = $(OBJDIR)/kernel.elf

all: $(TARGET)

$(OBJDIR)/%.o: $(SRCDIR)/%.c
	mkdir -p $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(TARGET):$(OBJS)
	$(LD) $(LDFLAGS) $(OBJS) -o $(TARGET)

run: $(TARGET)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 128M -nographic -serial mon:stdio -kernel $(TARGET)

debug: $(TARGET)
	qemu-system-aarch64 -M virt -cpu cortex-a72 -m 128M -nographic -serial mon:stdio -kernel $(TARGET) -s -S

clean:
	rm -rf $(OBJDIR)

.PHONY: all run debug clean