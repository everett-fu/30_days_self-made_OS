/*
 * Filename: mtask.c
 * Author: everett.fu
 * Created: 25-2-20
 * Last Modified: 25-2-20
 * Description:
 *
 * Functions:
 * - task_init: 初始化任务控制块
 * - task_alloc: 选择一个未使用的任务结构体，并初始化任务结构体
 * - task_run: 将任务添加到tasks的末尾，并将正在运行的任务数加1
 * - task_switch: 切换任务，公平时间片算法
 * Usage:
 */
#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;
int mt_tr;

// 关闭任务
#define TASK_FLAGS_CLOSE 0
// 启用任务
#define TASK_FLAGS_ALLOC 1
// 任务使用中
#define TASK_FLAGS_USING 2

/**
 * 初始化任务控制块
 * @param memman		内存管理结构体
 * @return				返回一个任务结构体的地址
 */
struct TASK * task_init(struct MEMMAN *memman) {
	int i;
	struct TASK *task;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) ADR_GDT;
	taskctl = (struct TASKCTL *)memman_alloc_4k(memman, sizeof(struct TASKCTL));
	// 初始化所有的任务
	for (i = 0; i < MAX_TASKS; i++) {
		taskctl->tasks0[i].flags = TASK_FLAGS_CLOSE;
		taskctl->tasks0[i].sel = (TASK_GDT0 + i) * 8;
		// 设置任务切换所用的寄存器与段设置
		set_segmdesc(gdt + TASK_GDT0 + i, 103, (int)&taskctl->tasks0[i].tss, AR_TSS32);
	}
	// 选择一个未使用的任务结构体
	task = task_alloc();
	// 将任务设置为活动中
	task->flags = TASK_FLAGS_USING;
	// 正在运行的任务数量
	taskctl->running_num = 1;
	// 当前运行的任务编号
	taskctl->now_task = 0;
	taskctl->tasks[0] = task;
	load_tr(task->sel);
	// 申请一个定时器作为任务定时器
	task_timer = timer_alloc();
	// 设置任务切换的时间
	timer_settime(task_timer, 2);
	return task;
}

/**
 * 选择一个未使用的任务结构体，并初始化任务结构体
 * @return				返回一个任务结构体的地址
 */
struct TASK * task_alloc(void) {
	int i;
	struct TASK *task;
	// 找到一个没有被使用的任务结构体
	for (i = 0; i < MAX_TASKS; i++) {
		// 如果这个任务结构体是没有被使用的，给她赋值，并启用
		if (taskctl->tasks0[i].flags == TASK_FLAGS_CLOSE) {
			task = &taskctl->tasks0[i];
			task->flags = TASK_FLAGS_ALLOC;
			task->tss.eflags = 0x00000202;
			task->tss.eax = 0;
			task->tss.ecx = 0;
			task->tss.edx = 0;
			task->tss.ebx = 0;
			task->tss.ebp = 0;
			task->tss.esi = 0;
			task->tss.edi = 0;
			task->tss.es = 0;
			task->tss.ds = 0;
			task->tss.fs = 0;
			task->tss.gs = 0;
			task->tss.ldtr = 0;
			task->tss.iomap = 0x40000000;
			return task;
		}
	}
	// 全部都在使用，没有找到空余的任务
	return 0;
}

/**
 * 将任务添加到tasks的末尾，并将正在运行的任务数加1
 * @param task			要运行任务的地址
 */
void task_run(struct TASK *task) {
	task->flags = TASK_FLAGS_USING;
	taskctl->tasks[taskctl->running_num] = task;
	taskctl->running_num++;
	return;
}

/**
 * 切换任务，公平时间片算法
 */
void task_switch(void) {
	timer_settime(task_timer, 2);
	// 如果有多个任务
	if (taskctl->running_num >= 2) {
		// 运行下一个任务
		taskctl->now_task++;
		// 当任务是最后一个任务，下个任务回到第一个任务
		if (taskctl->now_task == taskctl->running_num) {
			taskctl->now_task = 0;
		}
		// 跳转到下个任务执行
		farjmp(0, taskctl->tasks[taskctl->now_task]->sel);
	}
	return;
}
