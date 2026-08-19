
qemu :  vectorTable_qemu.c startup.c main.c initializer_qemu.c tasks.c syncTasks.c hardFaultHandler.c heap.c uart.c link_qemu.ld
	arm-none-eabi-gcc vectorTable_qemu.c startup.c main.c initializer_qemu.c tasks.c syncTasks.c hardFaultHandler.c heap.c uart.c -T link_qemu.ld -nostdlib -ffreestanding -mcpu=cortex-m3 -mthumb -o firmware_qemu.elf
	echo "qemu build complete"

stm32 : vectorTable_stm32.c startup.c main.c initializer_stm32.c tasks.c syncTasks.c hardFaultHandler.c heap.c usart.c clock.c link_stm32.ld
	arm-none-eabi-gcc vectorTable_stm32.c startup.c main.c initializer_stm32.c tasks.c syncTasks.c hardFaultHandler.c heap.c usart.c clock.c -T link_stm32.ld -nostdlib -ffreestanding -mcpu=cortex-m3 -mthumb -o firmware_stm32.elf
	echo "stm32 build complete"
