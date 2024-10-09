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

struct TIMERCTL timerctl;

/**
 * 初始化PIT
 */
void init_pit(void) {
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	timerctl.timeout = 0;
	return;
}

/**
 * 处理定时器中断
 * @param esp
 */
void inthandler20(int *esp) {
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	timerctl.count++;
	if (timerctl.timeout > 0) {
		timerctl.timeout--;
		if (timerctl.timeout == 0) {
			fifo8_put(timerctl.fifo, timerctl.data);
		}

	}
	return;
}

/**
 * 设置定时器超时时间与超时后显示的字符
 * @param timeout	超时时间
 * @param fifo		缓冲区
 * @param data		字符数据
 */
void settimer(unsigned int timeout, struct FIFO8 *fifo, unsigned char data) {
	int eflags;
	eflags = io_load_eflags();
	io_cli();
	timerctl.timeout = timeout;
	timerctl.fifo = fifo;
	timerctl.data = data;
	io_store_eflags(eflags);
	return;
}