# P5 RTOS Cortex-M3

A real time operating system written from scratch in C. No libraries, no HAL, no RTOS framework. Just the linker script, a vector table and everything else built up from there.

Started on QEMU emulating an ARM Cortex-M3 (mps2-an385 board) and now being ported to a real STM32F103C8T6.

**Status: in progress.** The QEMU side is done and working. The STM32 port is written but not tested yet since the hardware hasn't arrived.

## What it does

- Preemptive scheduling with priorities
- Context switching through PendSV
- Blocking mutexes built on LDREX/STREX
- Priority inheritance to fix priority inversion
- Heap allocator with block splitting and coalescing free
- Interrupt driven UART with a ring buffer
- Fault handler that decodes and prints what went wrong

## Priority inversion demo

This is the main thing the project demonstrates.

Priority inversion is when a high priority task gets stuck waiting on a lock held by a low priority task, and a medium priority task keeps preempting the low one so it never finishes and never releases. The high priority task ends up blocked behind something less important than itself. This is the bug that kept resetting the Mars Pathfinder rover in 1997.

I set up three tasks to make it happen on purpose:

- Task A, highest priority, wants the UART mutex
- Task B, medium priority, doesn't touch the mutex at all
- Task C, lowest priority, holds the mutex

Task C takes the lock. Task A tries to take it and blocks. Task B preempts C and runs, and A just sits there waiting.

Measured by counting how many times B ran before A finally got the lock:

| | B iterations before A runs |
|---|---|
| Without priority inheritance | 124 |
| With priority inheritance | 0 |

The fix is priority inheritance. When A blocks on a mutex held by C, C temporarily inherits A's priority so it beats B, finishes its work and releases. Then C drops back to its own priority.

## How it's built

**Boot.** Custom linker script places the vector table, code, data and bss. Reset handler copies .data from flash to RAM, zeroes .bss and calls main.

**Scheduling.** SysTick fires every 1ms and pends PendSV. PendSV is a naked function that pushes r4-r11 and lr, saves the stack pointer into the task struct, picks the next task and restores. Set to lowest priority so context switches only happen when nothing more urgent is running.

The scheduler walks the task array and picks the highest priority task that's ready. Blocked tasks get skipped. If nothing is runnable it prints a deadlock message and hangs instead of spinning forever.

**Mutexes.** LDREX and STREX for the atomic take. If the lock is held the task marks itself blocked and triggers a context switch instead of spinning, so it stops burning CPU. The release walks the task array and wakes anything waiting on that mutex.

Interrupts are masked with PRIMASK around the state change, otherwise there's a window where a task can be marked as waiting but not yet blocked, and a release in that window wakes nobody. That's a lost wakeup and the task sleeps forever.

**Heap.** Every block has an 8 byte header with a size and a free/used flag. Malloc walks from the start looking for a block big enough, splits it if the leftover is worth keeping, and returns a pointer past the header. Free flips the flag and then walks the whole heap merging any adjacent free blocks so the memory doesn't fragment into unusable pieces.

Sizes get rounded up to a multiple of 4 because the Cortex-M3 faults on unaligned word access.

**UART.** Started as a polling driver that sat in a loop watching a status bit. Now it queues characters into a 128 byte ring buffer and returns immediately, and a TX interrupt drains the buffer one character at a time. The buffer indices are shared between the task and the interrupt handler so PRIMASK protects them.

**Fault handler.** A naked stub checks bit 2 of the EXC_RETURN value in LR to work out whether the exception frame is on MSP or PSP, puts that address in r0 and branches to a normal C function. That function reads the stacked PC, plus CFSR and HFSR, and prints them over UART in hex.

## Bugs worth writing down

**The strex one.** Spent a whole session on this. The mutex faulted immediately and the terminal was completely blank. The disassembly showed `strex r3, r2, [r3]` with r3 used as both the result register and the address register, which ARM defines as UNPREDICTABLE and QEMU rejects as an undefined instruction.

The C source was fine. GCC assigned the same register to two operands because by default it assumes inline assembly reads all its inputs before writing any outputs, which isn't true for strex. The fix was one character: an earlyclobber `&` on the output constraint, which tells GCC not to share that register with an input.

**No fault handler.** The reason that took a whole session is that vector table slot 3 was zero, so the fault jumped to address 0, hit a value that isn't an instruction, faulted again at priority -1 with nowhere to escalate, and locked up with no output. After writing the handler the same bug printed its own diagnosis in three lines: PC 0x00000306, CFSR bit 16 UNDEFINSTR, HFSR bit 30 FORCED.

Build the diagnostic before you need it.

**Missing UART TX enable.** Very early on, nothing printed at all and everything looked correct. Bit 0 of the UART control register enables the transmitter and it isn't on by default.

**Wrong IRQ number.** The UART TX interrupt never fired. Turned out IRQ 0 on that board is receive and IRQ 1 is transmit, which meant the handler was also in the wrong vector table slot.

**Inverted priority comparison.** Priority inheritance did nothing at first. Lower number means higher priority in my scheme and I had the comparison the wrong way round, so the holder never inherited anything.

## Building and running

QEMU:

```
make qemu
qemu-system-arm -machine mps2-an385,accel=tcg -nographic -kernel firmware_qemu.elf
```

STM32:

```
make stm32
```

Then flash with ST-Link. Ctrl+A then X quits QEMU.

Two build targets because four files differ between the two: the linker script, the vector table, the UART driver and the initializer. Everything else is shared. Keeping the QEMU build alive means there's always a known good reference when something breaks on hardware.

## Known limitations

- Counting semaphores aren't implemented, only mutexes
- Priority inheritance doesn't handle nesting. If A blocks on C and C blocks on D, D doesn't inherit
- A task holding two mutexes drops back to its base priority when it releases either one
- The heap coalescing walks the entire heap on every free, which is O(n). Boundary tags would make it constant time but that costs a footer on every block and the heap is small
- The I2C read only implements the N byte sequence. Reading 1 or 2 bytes needs a different ACK sequence
- No DMA anywhere. Everything is interrupt driven or polled
- The STM32 port compiles but has never run on hardware

## What's next

- Bring up on a real STM32F103
- Custom motor control board, already designed and ordered
- Encoder input, PWM output and I2C current sensing
- A 1kHz PID speed control loop
- Capture the priority inversion on a logic analyzer through GPIO toggles instead of counting printed characters
