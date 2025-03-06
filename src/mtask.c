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
 * - task_switch: 切换任务，多重队列算法
 * - task_sleep: 任务休眠
 * - task_now: 返回当前任务的内存地址
 * - task_add: 向任务队列中添加一个任务
 * - task_remove: 从任务列表中删除一个任务
 * - task_switchsub: 决定任务切换的时候要切换到哪个任务列表
 *
 * Usage:
 */
#include "bootpack.h"

struct TASKCTL *taskctl;
struct TIMER *task_timer;



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
	// 初始化每个任务列表的正在运行数与正在运行任务编号
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		taskctl->level[i].running_num = 0;
		taskctl->level[i].now_task = 0;
	}
	// 选择一个未使用的任务结构体
	task = task_alloc();
	// 将任务设置为活动中
	task->flags = TASK_FLAGS_USING;
	// 将任务设置运行2ms
	task->priority = 2;
	// 将任务列表设置为列表0
	task->level = 0;
	// 将任务添加到任务列表
	task_add(task);
	// 切换任务列表
	task_switchsub();
	load_tr(task->sel);
	// 申请一个定时器作为任务定时器
	task_timer = timer_alloc();
	// 设置任务切换的时间
	timer_settime(task_timer, task->priority);
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
 * @param level			将任务添加到任务列表序号
 * @param priority 		运行的运行时间
 */
void task_run(struct TASK *task, int level, int priority) {
	// 如果任务列表编号小于0，不改变任务列表编号
	if (level < 0) {
		level = task->level;
	}
	// 如果任务的运行时间大于0
	if (priority > 0) {
		task->priority = priority;
	}
	// 如果任务是正在运行的任务，并且要改变任务列表，则移除之前任务列表的任务
	if (task->flags == TASK_FLAGS_USING && task->level != level) {
		// 如果移除以后，该任务的flag会变成TASK_FLAGS_ALLOC，下面的if也会执行
		task_remove(task);
	}
	if (task->flags != TASK_FLAGS_USING) {
		// 修改任务的任务列表编号
		task->level = level;
		// 将任务添加到任务列表
		task_add(task);
	}

	taskctl->lv_change = 1;
	return;
}

/**
 * 切换任务，多重队列算法
 */
void task_switch(void) {
	struct TASKLEVEL *t1 = &taskctl->level[taskctl->now_lv];
	struct TASK *new_task, *now_task = t1->tasks[t1->now_task];
	// 运行下一个任务
	t1->now_task++;
	// 当任务是最后一个任务，下个任务回到第一个任务
	if (t1->now_task == t1->running_num) {
		t1->now_task = 0;
	}
	// 如果需要改变任务队列
	if (taskctl->lv_change != 0) {
		// 找到要切换的任务队列
		task_switchsub();
		t1 = &taskctl->level[taskctl->now_lv];
	}
	// 获取新任务的地址
	new_task = t1->tasks[t1->now_task];
	// 设置新任务的运行时间
	timer_settime(task_timer, new_task->priority);
	// 如果任务列表只有一个任务，会出现下一个任务还是当前任务，判断下一个任务与当前的任务是否一致
	if (new_task != now_task) {
		// 跳转到下个任务执行
		farjmp(0, new_task->sel);
	}
	return;
}

/**
 * 任务休眠
 * @param task			要休眠的任务结构体的地址
 */
void task_sleep(struct TASK *task) {
	struct TASK *now_task;
	// 如果任务处于活动状态
	if (task->flags == TASK_FLAGS_USING) {
		// 获取当前正在执行任务的地址
		now_task = task_now();
		// 将任务从任务列表移除
		task_remove(task);
		// 如果正在执行的任务与要移除的任务是一个任务
		if (task == now_task) {
			// 找到下一个要执行的任务
			task_switchsub();
			// 获取下一个任务的地址
			now_task = task_now();
			// 跳转到下一个任务
			farjmp(0, now_task->sel);
		}
	}
	return;
}

/**
 * 返回当前任务的内存地址
 * @return				当前运行任务的地址
 */
struct TASK *task_now(void) {
	struct TASKLEVEL *t1 = &taskctl->level[taskctl->now_lv];
	return t1->tasks[t1->now_task];
}

/**
 * 向任务队列中添加一个任务
 * @param task			需要添加任务的地址
 */
void task_add(struct TASK *task) {
	struct TASKLEVEL *t1 = &taskctl->level[task->level];
	// 判断是否大于任务列表的最大值
	if (t1->running_num < MAX_TASKS_LV) {
		t1->tasks[t1->running_num] = task;
		t1->running_num++;
		task->flags = TASK_FLAGS_USING;
	}
	return;
}

/**
 * 从任务列表中删除一个任务
 * @param task			需要删除的任务
 */
void task_remove(struct TASK *task) {
	int i;
	struct TASKLEVEL *t1 = &taskctl->level[task->level];

	// 寻找task所在的位置
	for (i = 0; i < t1->running_num; i++) {
		if (t1->tasks[i] == task) {
			break;
		}
	}

	t1->running_num--;
	// 如果要休眠的任务在正在运行的任务的前面
	if (i < t1->now_task) {
		t1->now_task--;
	}
	// 万一要休眠的任务为最后一个，经过前面的步骤以后会出现now_task（要休眠的任务编号）>running_num（全部运行的任务数）
	if (t1->now_task >= t1->running_num) {
		t1->now_task = 0;
	}
	// 从要休眠的任务开始，将后面的任务向前平移一个位置，即覆盖要休眠的任务
	task->flags = TASK_FLAGS_ALLOC;
	for (; i < t1->running_num; i++) {
		t1->tasks[i] = t1->tasks[i + 1];
	}
}

/**
 * 决定任务切换的时候要切换到哪个任务列表
 */
void task_switchsub(void) {
	int i;
	for (i = 0; i < MAX_TASKLEVELS; i++) {
		if (taskctl->level[i].running_num > 0) {
			break;
		}
	}
	taskctl->now_lv = i;
	taskctl->lv_change = 0;
	return;
}

