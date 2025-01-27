/*
 * Filename: timer.c
 * Author: everett.fu
 * Created: 24-10-9
 * Last Modified: 24-10-9
 * Description:
 * 这个文件包含了定时器的实现。
 *
 * Functions:
 * - init_pit: 初始化PIT
 * - inthandler20: 处理定时器中断
 * - timer_alloc: 找到一个未使用的定时器，把该定时器状态改为已启动，并返回该定时器的地址
 * - timer_free: 将一个定时器关闭
 * - timer_init: 设置定时器超时时间与超时后显示的字符
 * - timer_settime: 设置定时器超时时间
 *
 * Usage:
 */

#include "bootpack.h"

#define PIT_CTRL 0x0043
#define PIT_CNT0 0x0040

// 关闭定时器
#define TIMER_FLAGS_CLOSE 0
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
	timerctl.next_timeout = 0xffffffff;
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = TIMER_FLAGS_CLOSE;
	}
	return;
}

/**
 * 处理定时器中断
 * @param esp
 */
void inthandler20(int *esp) {
	int i, j;
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	timerctl.count++;
	// 还没有到下一个超时的时间，直接退出
	if (timerctl.next_timeout > timerctl.count) {
		return;
	}
	for (i = 0; i < timerctl.using; i++) {
		// 这里的循环超时时间是从小到大排序的，因此当超时时间大于当前时间的时候，i就是已经超时的定时器个数。
		// 如果当前定时未超时，则跳出循环。
		if (timerctl.timers[i]->timeout > timerctl.count) {
			break;
		}
		// 当定时器超时的时候，设置定时器状态，并向相应的缓冲区输出数据
		timerctl.timers[i]->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timerctl.timers[i]->fifo, timerctl.timers[i]->data);
	}
	// 减去已经超时的定时器
	timerctl.using -= i;
	// 将已经超时的定时器移出正在使用的定时器序列表
	for (j = 0; j < timerctl.using; j++) {
		timerctl.timers[j] = timerctl.timers[i + j];
	}
	// 当还有使用的定时器的时候，将下一个超时时间设置成最短的超时时间。
	if (timerctl.using > 0) {
		timerctl.next_timeout = timerctl.timers[0]->timeout;
	}
	else {
		timerctl.next_timeout = 0xffffffff;
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
		if (timerctl.timers0[i].flags == TIMER_FLAGS_CLOSE) {
			timerctl.timers0[i].flags = TIMER_FLAGS_ALLOC;
			return &timerctl.timers0[i];
		}
	}
	return 0;
}

/**
 * 将一个定时器关闭
 * @param timer		要关闭的定时器地址
 */
void timer_free(struct TIMER *timer) {
	timer->flags = TIMER_FLAGS_CLOSE;
	return;
}

/**
 * 设置定时器超时时间与超时后显示的字符
 * @param timer		要设置的定时器的地址
 * @param fifo		要写入的缓冲区
 * @param data		要写入的数据
 */
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data) {
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
	int e, i;
	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	// 关闭中断
	e = io_load_eflags();
	io_cli();
	// 因为这里的数据是已经排序好的，这里使用简单插入排序就可以了，不需要使用复杂的排序算法
	// 前面的0~using-1位已经排好了，从后到前，找到一个小于该超时时间的位置
	for (i = timerctl.using - 1; i >= 0 && timerctl.timers[i]->timeout > timer->timeout; i--) {
		timerctl.timers[i + 1] = timerctl.timers[i];
	}
	timerctl.using++;
	timerctl.timers[i + 1] = timer;
	timerctl.next_timeout = timerctl.timers[0]->timeout;

	// 开启中断
	io_store_eflags(e);
	return;
}
