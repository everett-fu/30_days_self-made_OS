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
	struct TIMER *t;
	io_out8(PIT_CTRL, 0x34);
	io_out8(PIT_CNT0, 0x9c);
	io_out8(PIT_CNT0, 0x2e);
	timerctl.count = 0;
	// 将所有的定时器都置为为使用
	for (i = 0; i < MAX_TIMER; i++) {
		timerctl.timers0[i].flags = TIMER_FLAGS_CLOSE;
	}
	// 获取一个定时器，作为尾节点，该节点的值无穷大
	t = timer_alloc();
	t->timeout = 0xffffffff;
	t->flags = TIMER_FLAGS_USING;
	t->next_timer = 0;

	// 将尾节点置为第一个节点，之后的所有节点都要插入到这个节点之前
	timerctl.timer_head = t;
	timerctl.next_timeout = 0xffffffff;
	return;
}

/**
 * 处理定时器中断
 * @param esp
 */
void inthandler20(int *esp) {
	struct TIMER *timer;
	io_out8(PIC0_OCW2, 0x60);	/* 把IRQ-00信号接收完了的信息通知给PIC */
	timerctl.count++;
	// 还没有到下一个超时的时间，直接退出
	if (timerctl.next_timeout > timerctl.count) {
		return;
	}
	// 将第一个定时器赋值给timer
	timer = timerctl.timer_head;
	for (;;) {
		// 如果当前定时未超时，则跳出循环。
		if (timer->timeout > timerctl.count) {
			break;
		}
		// 超时
		// 当定时器超时的时候，设置定时器状态，并向相应的缓冲区输出数据
		//timer_free(timer);
		timer->flags = TIMER_FLAGS_ALLOC;
		fifo32_put(timer->fifo, timer->data);
		// 将下一个定时器的地址赋值给timer
		timer = timer->next_timer;
	}
	timerctl.timer_head = timer;
	timerctl.next_timeout = timer->timeout;
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
	int e;
	struct TIMER *t, *t_next;

	timer->timeout = timeout + timerctl.count;
	timer->flags = TIMER_FLAGS_USING;
	// 关闭中断
	e = io_load_eflags();
	io_cli();

	t = timerctl.timer_head;
	// 要插入到最前面
	if (timer->timeout <= t->timeout) {
		timerctl.timer_head = timer;
		timer->next_timer = t;
		timerctl.next_timeout = timer->timeout;
		// 开启中断
		io_store_eflags(e);
		return;
	}
	// 因为有一个特别大的尾节点，因此如果上面的if判断不是在最前面，那应该插到第二个(包含)至倒数第二个（包含）之间任何一个
	// 所以t->next_timer第一次执行一定不是空，而且一定不会插入到最后一个节点
	for(;;){
		t_next = t->next_timer;
		if (timer->timeout <= t_next->timeout) {
			timer->next_timer = t_next;
			t->next_timer = timer;
			// 开启中断
			io_store_eflags(e);
			return;
		}
		t = t_next;
	}
}
