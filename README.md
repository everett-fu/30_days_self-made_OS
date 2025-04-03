The original version of this article is the Chinese version, and the English version is translated by AI, if there is a translation error, please contact me.

**Read other versions: [English](README.md), [中文](README_zh.md).**

# NubulaOS
This project is an implementation, porting, and optimization of the HaribOS operating system from Hideki Kawai’s book *30 Days to Build Your Own Operating System*.

## Project Introduction
The original code is HaribOS (Doll OS), written by Hideki Kawai. For teaching convenience, the author used a set of non-standard development tools, making the operating system nearly impossible to run on any non-Windows platform.  
Although the author provided a Docker runtime method and a Linux porting package, these are still not standard development tools, making the development process difficult to integrate into existing OS development workflows.  
Additionally, HaribOS was written a long time ago, using floppy disks + MBR + BIOS. In today's world, floppy disks are almost obsolete as a boot medium, while more modern boot methods involve USB drives/CDs + GPT + UEFI.

This project will rewrite parts of the original code using NASM + GCC + QEMU and replace some algorithms with more modern implementations. I hope this rewrite can provide some help to future developers. Therefore, I named this project *NubulaOS*, meaning "nebula," symbolizing clearing away the fog and offering guidance for those who follow.

The project will first complete Hideki Kawai's original code and then proceed with refactoring, rewriting, and optimization. Currently, the content from Day 0 to Day 21 has been completed.

## Project Features
+ Uses GCC and NASM as the toolchain, supporting cross-platform development
+ Boots via USB + GPT + UEFI
+ Optimized algorithms
+ No dependency on the author's custom HRB file format, supports ELF file format
+ Simple implementation of some C standard library functions (`libc` file)
+ Header files are split for better code organization
+ Supports Chinese keyboard input

## Environment Dependencies
+ `nasm`
+ `gcc`
+ `binutils`
+ `qemu-system-i386`
+ `mtools`

### Project directory structure
├─`build`: intermediate files<br>
├─`images`: image files<br>
├─`src`: project source code<br>
└─`z_tools`: auxiliary build tools<br>

### Running the Project
At this stage, the project still follows Hideki Kawai's original code and has not undergone tool standardization. Therefore, the original development tools are still required, and the project is currently limited to Windows platforms.  
The specific usage process is as follows:
1. Clone this repository
2. Download Hideki Kawai's *z_tools* package and place it in the project directory (at the same level as src)
3. Open `!cons_nt.bat` in src and enter `make run` to launch QEMU
4. To exit, simply close QEMU

## Project Progress

+ [X] Day 1: Hello World
+ [X] Day 2: Assembly and Makefile
+ [X] Day 3: Entering 32-bit Mode
+ [X] Day 4: Display Output
+ [X] Day 5: GDT/IDT
+ [X] Day 6: Interrupt Handling
+ [X] Day 7: FIFO
+ [X] Day 8: Mouse and 32-bit Mode
+ [X] Day 9: Memory Management
+ [X] Day 10: Window Stacking
+ [X] Day 11: Window Handling
+ [X] Day 12: Timer 1
+ [X] Day 13: Timer 2
+ [X] Day 14: Keyboard Input
+ [X] Day 15: Multitasking 1
+ [X] Day 16: Multitasking 2
+ [X] Day 17: Command Line Window
+ [X] Day 18: Command Line Commands
+ [X] Day 19: Applications
+ [X] Day 20: API
+ [X] Day 21: OS Protection
+ [ ] Day 22: C Applications (Converted to ELF Format)
+ [ ] Day 23: Application Graphics Processing
+ [ ] Day 24: Window Operations
+ [ ] Day 25: More Windows
+ [ ] Day 26: Window Operation Optimization
+ [ ] Day 27: LDT and Libraries (Different from the Book's Approach)
+ [ ] Day 28: File Operations and Text Display (Excluding Japanese Display)

## Outstanding Issues
+ [ ] Complete Hideki Kawai’s original source code
+ [ ] Use GCC and NASM as the toolchain for cross-platform development
+ [ ] Boot via USB + GPT + UEFI
+ [ ] Optimize algorithms
+ [ ] Remove dependency on the author's custom HRB file format, support ELF file format
+ [ ] Provide a simple implementation of some C standard library functions (`libc` file)
+ [ ] Refactor header files for clearer structure
+ [ ] Support Chinese keyboard input  
