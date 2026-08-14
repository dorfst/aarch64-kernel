make EXCLUDE="$1";
qemu-system-aarch64 -M virt -cpu cortex-a72 -m 128M -nographic -serial mon:stdio -kernel ./obj/kernel.elf -s -S;
