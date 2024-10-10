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

// 启用定时器
#define TIMER_FLAGS_ALLOC 1
// 定时器运行中
#define TIMER_FLAGS_USING 2

struct TIMERCTL timerctl;

/**
 * 初始化PIT
 */
void init_pit(void) {
	int i;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timer[i].flags = 0;
	}
	return;
}

/**
 * 处理定时器中断
 * @param esp
 */
void inthandler20(int *esp) {
	int i;
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	timerctl.count++;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timer[i].flags == TIMER_FLAGS_USING) {
			timerctl.timer[i].timeout--;
			if (timerctl.timer[i].timeout == 0) {
				fifo8_put(timerctl.timer[i].fifo, timerctl.timer[i].data);
			}

		}

	}
	return;
}

/**
 * 找到一个未使用的定时器，把该定时器状态改为已启动，并返回该定时器的地址
 * @return		找到定时器返回定时器地址，未找到返回0
 */
struct TIMER * timer_alloc(void) {
	int i ;
	for (i = 0; i < MAX_TIMER; i++) {
		if (timerctl.timer[i].flags == 0) {
			timerctl.timer[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timer[i];
		}
	}
	return 0;
}

/**
 * 将一个定时器关闭
 * @param timer		要关闭的定时器地址
 */
void timer_free(struct TIMER *timer) {
	timer->flags = 0;
	return;
}

/**
 * 设置定时器超时时间与超时后显示的字符
 * @param timer		要设置的定时器的地址
 * @param fifo		要写入的缓冲区
 * @param data		要写入的数据
 */
void timer_init(struct TIMER *timer, struct FIFO8 *fifo, unsigned char data) {
	timer->fifo = fifo;
	timer->data = data;
	return;
}

/**
 * 设置定时器超时时间
 * @param timer		要设置的定时器的地址
 * @param timeout	要设置的定时超时时间
 */
void timer_settime(struct TIMER *timer, unsigned int timeout) {
	timer->timeout = timeout;
	timer->flags = TIMER_FLAGS_USING;
	return;
}




