/*
 * Filename: console.c
 * Author: everett.fu
 * Created: 25-3-25
 * Last Modified: 25-3-25
 * Description:
 * 这个文件是命令行窗口
 *
 *Functions:
 * - console_task: 任务b
 * - cons_newline: 换行函数
 * - cons_putchar: 显示字符
 * - cons_runcmd: 运行命令
 * - cmd_free: 显示剩余内存
 * - cmd_clear: 清屏
 * - cmd_ls: 显示文件
 * - cmd_cat: 显示文件内容
 * - int cmd_app: 运行应用程序
 *
 * Usage:
 */

#include <stdio.h>
#include <string.h>
#include "bootpack.h"

/**
 * 任务b
 * @param sht_back		要显示的图层地址
 * @param memtotal		内存地址
 */
void console_task(struct SHEET *sheet, unsigned int memtotal){
	// 界面刷新定时器
	struct TIMER *timer;
	// 获取当前任务的地址
	struct TASK *task = task_now();
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	// 缓冲区数据
	int fifobuf[128];
	// 申请地址空间，用于保存fat表
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);
	// 临时变量
	int i;
	struct CONSOLE cons;
	cons.sht = sheet;
	cons.cur_x = 8;
	cons.cur_y = 28;
	cons.cur_c = -1;
	*((int*) 0x0fec) = (int)&cons;
	// 临时变量，用于存储字符
	char cmdline[30];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	// 显示提示符
	cons_putchar(&cons, '>', 1);

	for (;;) {
		io_cli();
		// 如果没有数据，则休眠
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		}
		else {
			i = fifo32_get(&task->fifo);
			io_sti();
			// 光标定时器
			if (i <= 1) {
				if (i == 1) {
					timer_init(timer, &task->fifo, 0);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				}
				else {
					timer_init(timer, &task->fifo, 1);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			// 打开光标
			else if (i == 2) {
				cons.cur_c = COL8_FFFFFF;
			}
			// 关闭光标
			else if (i == 3) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
				cons.cur_c = -1;
			}
			// 如果有键盘输入，则显示键盘输入
			if (i >=256 && i <= 511) {
				// 退格符
				if (i == 8 + 256) {
					if (cons.cur_x > 16) {
						// 把光标的位置变成背景颜色，再改上一个字符的颜色
						cons_putchar(&cons, ' ', 0);
						cons.cur_x -=8;
					}
				}
				// 回车键
				else if (i == 10 + 256) {
					// 将光标位置变成背景颜色以后换行
					cons_putchar(&cons, ' ', 0);
					cmdline[cons.cur_x / 8 - 2] = 0;
					cons_newline(&cons);
					// 运行命令
					cons_runcmd(cmdline, &cons, fat, memtotal);
					cons_putchar(&cons, '>', 1);
				}
				// 一般字符
				else {
					if ( cons.cur_x < 240) {
						cmdline[cons.cur_x / 8 - 2] = i - 256;
						cons_putchar(&cons, i - 256, 1);
					}
				}
			}
			// 重新显示光标
			if (cons.cur_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cons.cur_c, cons.cur_x, cons.cur_y, cons.cur_x + 7, cons.cur_y + 15);
			}
			sheet_refresh(sheet, cons.cur_x, cons.cur_y, cons.cur_x + 8, cons.cur_y + 16);
		}
	}
}

/**
 * 换行函数
 * @param cons		命令窗口参数
 */
void cons_newline(struct CONSOLE *cons) {
	int x, y;
	struct SHEET *sheet = cons->sht;
	// 如果没有超出窗口纵向范围
	if (cons->cur_y < 28 + 112) {
		cons->cur_y += 16;
	}
	// 超出窗口纵向范围
	else {
		// 将窗口内容向上移动
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		// 清除最后一行
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		// 显示命令提示符>
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	cons->cur_x = 8;
	// 返回当前光标纵坐标
	return;
}

/**
 * 显示字符
 * @param cons		命令窗口参数
 * @param chr		字符
 * @param move		是否将光标移动到下一个位置，除退格符以外，输出字符一般要挪到下一个位置
 */
void cons_putchar(struct CONSOLE *cons, char chr, char move) {
	char s[2];
	s[0] = chr;
	s[1] = 0;
	// 制表符
	if (s[0] == 0x09) {
		for (;;) {
			putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, " ", 1);
			cons->cur_x += 8;
			// 如果到界面最后，进行换行
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
			// 如果光标位置是32的倍数，则跳出循环
			// 减8是因为光标位置是从8开始的，开始有8个像素是窗口的宽度
			if (((cons->cur_x - 8) & 0x1f) == 0) {
				break;
			}
		}
	}
	// 换行符
	else if (s[0] == 0x0a) {
		cons_newline(cons);
	}
	// 回车符
	else if (s[0] == 0x0d) {
	}
	// 一般字符
	else {
		putfonts8_asc_sht(cons->sht, cons->cur_x, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 1);
		if (move != 0) {
			cons->cur_x += 8;
			// 如果到界面最后，进行换行
			if (cons->cur_x == 8 + 240) {
				cons_newline(cons);
			}
		}
	}
	return;
}

/**
 * 运行命令
 * @param cmdline		运行命令
 * @param cons			命令窗口参数
 * @param fat			fat表
 * @param memtotal		内存地址
 */
void cons_runcmd(char *cmdline, struct CONSOLE *cons, int *fat, unsigned int memtotal) {
	if (cmdline[0] == 0) {
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "Nothing!", 8);
		cons_newline(cons);
		return;
	}
	// free命令，显示剩余内存
	else if (strcmp(cmdline, "free") == 0) {
		cmd_free(cons, memtotal);
	}
	// clear命令，清屏
	else if (strcmp(cmdline, "clear") == 0) {
		cmd_clear(cons);
	}
	// 查看文件
	else if (strcmp(cmdline, "ls") == 0) {
		cmd_ls(cons);
	}
	// cat命令，只比较前四个字符，后面的为文件名
	else if (strncmp(cmdline, "cat ", 4) == 0) {
		cmd_cat(cons, fat, cmdline);
	}
	else if (cmdline[0] != 0) {
		// 不是命令
		if (cmd_app(cons, fat, cmdline) == 0) {
			putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "command not found!", 19);
			cons_newline(cons);
			cons_newline(cons);
		}
	}
	return;
}

/**
 * 显示剩余内存
 * @param cons			命令行窗口参数
 * @param memtotal		内存地址
 */
void cmd_free(struct CONSOLE *cons, unsigned int memtotal) {
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	char s[30];
	sprintf(s, "     total    used    free");
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(cons);

	sprintf(s, "Mem: %dMB     %dKB  %dKB", memtotal / (1024 * 1024), memtotal / 1024 - memman_total(memman) / 1024 , memman_total(memman) / 1024);
	putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
	cons_newline(cons);
	cons_newline(cons);
	return;
}

/**
 * 清屏
 * @param cons		命令行窗口参数
 */
void cmd_clear(struct CONSOLE *cons) {
	int x, y;
	struct SHEET *sheet = cons->sht;
	for (y = 28; y < 28 + 128; y++) {
		for (x = 8; x < 8 + 240; x++) {
			sheet->buf[x + y * cons->sht->bxsize] = COL8_000000;
		}
	}
	sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	cons->cur_y = 28;
	return;
}

/**
 * 显示文件
 * @param cons		命令行窗口参数
 */
void cmd_ls(struct CONSOLE *cons) {
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	int x, y;
	char s[30];
	// 最多存放224个文件
	for (x = 0; x < 224; x++) {
		// 这段不含文件信息
		if (finfo[x].name[0] == 0x00) {
			break;
		}
			// 文件没有被删除
		else if (finfo[x].name[0] != 0xe5) {
			// 不是归档文件或者目录
			if ((finfo[x].type & 0x18) == 0)  {
				sprintf(s, "filename.ext %7d", finfo[x].size);
				for (y = 0; y < 8; y++) {
					s[y] = finfo[x].name[y];
				}
				s[9] = finfo[x].ext[0];
				s[10] = finfo[x].ext[1];
				s[11] = finfo[x].ext[2];
				putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, s, 30);
				cons_newline(cons);
			}
		}
	}
	cons_newline(cons);
	return;
}

/**
 * 显示文件内容
 * @param cons			命令行窗口参数
 * @param fat			fat表
 * @param cmdline		文件名
 */
void cmd_cat(struct CONSOLE *cons, int *fat, char *cmdline) {
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct FILEINFO *finfo = file_search(cmdline + 4, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	char *p;
	int i;
	// 找到文件
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		// 加载文件
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		for (i = 0; i < finfo->size; i++) {
			cons_putchar(cons, p[i], 1);
		}
		// 释放内存
		memman_free_4k(memman, (int)p, finfo->size);
	}
	// 没有找到文件
	else {
		putfonts8_asc_sht(cons->sht, 8, cons->cur_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
		cons_newline(cons);
	}
	cons_newline(cons);
	return;
}


/**
 * 运行应用程序
 * @param cons			命令行窗口参数
 * @param fat			fat表
 * @param cmdline		需要运行的命令
 */
int cmd_app(struct CONSOLE *cons, int *fat, char *cmdline) {
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;
	struct FILEINFO *finfo;
	char name[18], *p;
	int i;

	// 将命令名复制到文件名
	for (i = 0; i < 13; i++) {
		// 在ASCII码中，小于空格的一般为控制字符，无实际意义
		if (cmdline[i] <= ' ') {
			break;
		}
		name[i] = cmdline[i];
	}
	name[i] = 0;

	finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	// 如果找不到文件，并且最后不是.开始，加上.hrb以后再查找一遍
	if (finfo == 0 && name[i - 1] != '.') {
		name[i] = '.';
		name[i + 1] = 'H';
		name[i + 2] = 'R';
		name[i + 3] = 'B';
		name[i + 4] = 0;
		finfo = file_search(name, (struct FILEINFO *)(ADR_DISKIMG + 0x002600), 224);
	}
	// 找到文件
	if (finfo !=0) {
		p = (char *)memman_alloc_4k(memman, finfo->size);
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		set_segmdesc(gdt + 1003, finfo->size, (int)p, AR_CODE32_ER);
		farcall(0, 1003 * 8);
		cons_newline(cons);
		memman_free_4k(memman, (int)p, finfo->size);
		return 1;
	}
	return 0;
}