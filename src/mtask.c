/*
 * Filename: mtask.c
 * Author: everett.fu
 * Created: 25-2-20
 * Last Modified: 25-2-20
 * Description:
 *
 * Functions:
 * - mt_init: 初始化任务定时器与任务编号
 * - mt_taskswitch: 切换任务
 *
 * Usage:
 */
#include "bootpack.h"

struct TIMER *mt_timer;
int mt_tr;

/**
 * 初始化任务定时器与任务编号
 */
void mt_init(void) {
	mt_timer = timer_alloc();
	timer_settime(mt_timer, 2);
	mt_tr = 3 * 8;
	return;
}

/**
 * 切换任务
 */

void mt_taskswitch(void) {
	if (mt_tr == 3 * 8) {
		mt_tr = 4 * 8;
	}
	else if (mt_tr == 4 * 8) {
		mt_tr = 3 * 8;
	}
	timer_settime(mt_timer, 2);
	farjmp(0, mt_tr);
	return;
}