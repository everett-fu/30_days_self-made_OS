/*
 * Filename: timer.c
 * Author: everett.fu
 * Created: 24-10-9
 * Last Modified: 24-10-9
 * Description:
 *   This file contains the implementation of timer. The program
 *   demonstrates basic functionality and serves as an example.
 *
 * Functions:
 *   - main: The entry point of the program.
 *   - ${Function1}: Description of the function.
 *   - ${Function2}: Description of the function.
 *
 * Usage:
 *   To compile: gcc -o timer timer.c
 *   To run: ./ timer
 */

#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

/**
 * 初始化PIT
 */
void init_pit(void) {
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	return;
}

/**
 * 处理定时器中断
 * @param esp
 */
void inthandler20(int *esp) {
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	return;
}