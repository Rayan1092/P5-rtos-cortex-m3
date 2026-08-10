firmware.elf : link.ld main.c vectorTable.c startup.c initializer.c tasks.c syncTasks.c hardFaultHandler.c heap.c
	arm-none-eabi-gcc vectorTable.c startup.c main.c initializer.c tasks.c syncTasks.c hardFaultHandler.c heap.c -T link.ld -nostdlib -ffreestanding -mcpu=cortex-m3 -mthumb -o firmware.elf
	echo "build executed!"
