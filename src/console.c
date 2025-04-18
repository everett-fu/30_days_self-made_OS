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
 * - cons_putstr: 显示完整字符串
 * - cons_putstr_length: 显示指定长度字符串
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
	cons.timer = timer_alloc();
	timer_init(cons.timer, &task->fifo, 1);
	timer_settime(cons.timer, 50);

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
					timer_init(cons.timer, &task->fifo, 0);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_FFFFFF;
					}
				}
				else {
					timer_init(cons.timer, &task->fifo, 1);
					if (cons.cur_c >= 0) {
						cons.cur_c = COL8_000000;
					}
				}
				timer_settime(cons.timer, 50);
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
			cons_putstr(cons, "Command not found!\n\n");
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
	sprintf(s, "\t total\tused\tfree\n");
	cons_putstr(cons, s);
	sprintf(s, "Mem: %dMB\t%dKB\t%dKB\n\n", memtotal / (1024 * 1024), memtotal / 1024 - memman_total(memman) / 1024 , memman_total(memman) / 1024);
	cons_putstr(cons, s);
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
				sprintf(s, "filename.ext %7d\n", finfo[x].size);
				for (y = 0; y < 8; y++) {
					s[y] = finfo[x].name[y];
				}
				s[9] = finfo[x].ext[0];
				s[10] = finfo[x].ext[1];
				s[11] = finfo[x].ext[2];
				cons_putstr(cons, s);
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
	// 找到文件
	if (finfo != 0) {
		p = (char *) memman_alloc_4k(memman, finfo->size);
		// 加载文件
		file_loadfile(finfo->clustno, finfo->size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
		cons_putstr_length(cons, p, finfo->size);
		// 释放内存
		memman_free_4k(memman, (int)p, finfo->size);
	}
	// 没有找到文件
	else {
		cons_putstr(cons, "File not found.\n");
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
	struct TASK *task = task_now();
	struct SHTCTL *shtctl;
	struct SHEET *sht;
	char name[18], *p, *q;
	int i;

	int segsiz, datsiz, esp, dathrb;

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
		// 判断是不是应用程序
		if (finfo->size >= 36 && strncmp(p + 4, "Hari", 4) == 0 && *p == 0x00) {
			// 获取应用程序数据段大小
			segsiz = *((int *) (p + 0x0000));
			// esp是堆栈指针
			esp = *((int *) (p + 0x000c));
			// 数据段的字节数
			datsiz = *((int *) (p + 0x0010));
			// 数据段的起始地址
			dathrb = *((int *) (p + 0x0014));
			// 申请数据段
			q = (char *)memman_alloc_4k(memman, segsiz);
			*((int *)0xfe8) = (int)q;
			//1003作为应用程序代码段，1004作为数据段
			set_segmdesc(gdt + 1003, finfo->size - 1, (int)p, AR_CODE32_ER + 0x60);
			set_segmdesc(gdt + 1004, segsiz - 1, (int)q, AR_DATA32_RW + 0x60);
			// 将数据段的内容复制到申请的内存中
			for (i = 0; i < datsiz; i++) {
				q[esp + i] = p[dathrb + i];
			}
			// 调用应用程序
			start_app(0x1b, 1003 * 8, esp, 1004 * 8, &(task->tss.esp0));
			shtctl = (struct SHTCTL *)*((int *)0x0fe4);
			// 关闭程序时检查图层有没有关闭
			for (i = 0; i < MAX_SHEETS; i++) {
				sht = &(shtctl->sheets0[i]);
				if ((sht->flags & 0x11) == 0x11 && sht->task == task) {
					sheet_free(sht);
				}
			}
			// 关闭程序时判断定时器自动关闭定时器
			timer_cancelall(&task->fifo);
			// 释放数据段
			memman_free_4k(memman, (int) q, segsiz);
		} else {
			cons_putstr(cons, ".hrb file format error.\n");
		}
		memman_free_4k(memman, (int)p, finfo->size);
		cons_newline(cons);
		return 1;
	}
	return 0;
}

/**
 * 显示末尾为0的完整字符串
 * @param cons		命令行窗口参数
 * @param s			字符串
 */
void cons_putstr(struct CONSOLE *cons,char *s) {
	for (; *s != 0; s++) {
		cons_putchar(cons, *s, 1);
	}
	return;
}

/**
 * 显示指定长度的字符串
 * @param cons		命令行窗口参数
 * @param s			字符串
 * @param l			需要显示字符的长度
 */
void cons_putstr_length(struct CONSOLE *cons, char *s, int l) {
	int i;
	for (i = 0; i < l; i++) {
		cons_putchar(cons, s[i], 1);
	}
	return;
}

/**
 * 字符显示api函数，给出不同的参数调用不同的API
 * @param edi		图层高度
 * @param esi		图层宽度
 * @param ebp		寄存器ebp
 * @param esp		寄存器esp
 * @param ebx		字符串地址，图层缓冲区
 * @param edx		功能号
 * @param ecx		字符长度，窗口标题
 * @param eax		字符编码，透明色
 * @return 			地址值
 */
int *hrb_api(int edi, int esi, int ebp, int esp, int ebx, int edx, int ecx, int eax) {
	int ds_base = *((int*)0xfe8);
	struct TASK *task = task_now();
	struct CONSOLE *cons = (struct CONSOLE *)*((int *)0x0fec);
	struct SHTCTL *shtctl = (struct SHT_CTL *)*((int *)0x0fe4);
	struct SHEET *sht;
	int *reg = &eax + 1;
	/*
	 * reg[0] = edi, reg[]1] = esi, reg[2] = ebp, reg[3] = esp
	 * reg[4] = ebx, reg[5] = edx, reg[6] = ecx, reg[7] = eax
	 */
	char s[30];
	// 显示单个字符
	if (edx == 1) {
		// AL=字符编码
		cons_putchar(cons, eax & 0xff, 1);
	}
	// 显示末尾为0的完整字符串
	else if (edx == 2) {
		// EBX=字符串地址
		cons_putstr(cons, (char *)ebx + ds_base);
		sprintf(s, "%08X\n", ebx);
		cons_putstr(cons, s);
	}
	// 显示指定长的的字符串
	else if (edx == 3) {
		// EBX=字符串地址，ECX=字符串长度
		cons_putstr_length(cons, (char *)ebx + ds_base, ecx);
	}
	// 结束应用程序
	else if (edx == 4) {
		return &(task->tss.esp0);
	}
	// 创建窗口
	else if (edx == 5) {
		sht = sheet_alloc(shtctl);
		sht->task = task;
		sht->flags |= 0x10;
		sheet_setbuf(sht, (char *)ebx + ds_base, esi, edi, eax);
		make_window8((char *)ebx + ds_base, esi, edi, (char *)ecx + ds_base, 0);
		sheet_slide(sht, (shtctl->xsize - esi) / 2, (shtctl->ysize - edi) / 2);
		sheet_updown(sht, shtctl->top - 1);
		reg[7] = (int)sht;
	}
	// 窗口上显示字符api
	else if (edx == 6) {
		// 按2的倍数取整
		sht = (struct SHEET *)(ebx & 0xfffffffe);
		putfonts8_asc(sht->buf, sht->bxsize, esi, edi, eax, (char *)ebp + ds_base);
		// 判断最低位是否为0，如果是0，则为偶数，则刷新窗口
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + ecx * 8, edi + 16);
		}
	}
	// 窗口上描绘矩形api
	else if (edx ==7) {
		sht = (struct SHEET *)(ebx & 0xfffffffe);
		boxfill8(sht->buf, sht->bxsize, ebp, eax, ecx, esi, edi);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	// 应用程序初始化栈api
	// EDX = 8
	// EBX = memman的地址
	// EAX = memman所管理的内存空间的起始地址
	// ECX = memman所管理的内存空间的字节数
	else if (edx == 8) {
		memman_init((struct MEMMAN *)(ebx + ds_base));
		// 将内存空间字节数对齐到16字节
		ecx &= 0xfffffff0;
		// 初始化所有的内存空间
		memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
	}
	// 应用程序栈分配api
	// EDX = 9
	// EBX = memman的地址
	// ECX = 需要请求的字节数
	// EAX = 分配到的内存空间地址
	else if (edx == 9) {
		// 将内存空间字节数对齐到16字节（向上取整）
		ecx = (ecx + 0x0f) & 0xfffffff0;
		reg[7] = memman_alloc((struct MEMMAN *)(ebx + ds_base), ecx);
	}
	// 应用程序栈释放api
	// EDX = 10
	// EBX = memman的地址
	// EAX = 需要释放的内存空间地址
	// ECX = 需要释放的字节数
	else if (edx == 10) {
		ecx = (ecx +0x0f) & 0xfffffff0;
		memman_free((struct MEMMAN *)(ebx + ds_base), eax, ecx);
	}
	// 应用程序画点api
	// EDX = 11
	// EBX = 窗口句柄
	// ESI = 显示的x坐标
	// EDI = 显示的y坐标
	// EAX = 色号
	else if (edx == 11) {
		sht = (struct SHEET *)(ebx & 0xfffffffe);
		sht->buf[sht->bxsize * edi + esi] = eax;
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, esi, edi, esi + 1, edi + 1);
		}
	}
	// 刷新窗口api
	// EDX = 12
	// EBX = 窗口句柄
	// EAX = 刷新区域左上角x坐标
	// ECX = 刷新区域左上角y坐标
	// ESI = 刷新区域的右下角x坐标
	// EDI = 刷新区域的右下角y坐标
	else if (edx == 12) {
		sht = (struct SHEET *)ebx;
		sheet_refresh(sht, eax, ecx, esi, edi);
	}
	// 画直线api
	// EDX = 13
	// EBX = 窗口句柄
	// EAX = x0
	// ECX = y0
	// ESI = x1
	// EDI = y1
	// EBP = 色号
	else if (edx == 13) {
		sht = (struct SHEET *)(ebx & 0xfffffffe);
		hrb_api_linewin(sht, eax, ecx, esi, edi, ebp);
		if ((ebx & 1) == 0) {
			sheet_refresh(sht, eax, ecx, esi + 1, edi + 1);
		}
	}
	// 关闭窗口api
	// EDX = 14
	// EBX = 窗口句柄
	else if (edx == 14) {
		sheet_free(((struct SHEET *)ebx));
	}
	// 键盘输入api
	// EDX = 15
	// EAX = 0 没有键盘输入时返回-1，不休眠
	//		 1 休眠直到发生键盘输入
	// EAX = 输入的字符编码
	else if (edx == 15) {
		int i;
		for (;;) {
			io_cli();
			// fifo为空
			if (fifo32_status(&task->fifo) == 0) {
				// 休眠
				if (eax != 0) {
					task_sleep(task);
				}
				// 不休眠
				else {
					io_sti();
					reg[7] = -1;
					return 0;
				}
			}
			i = fifo32_get(&task->fifo);
			io_sti();
			// 光标定时器
			if (i <= 1) {
				timer_init(cons->timer, &task->fifo, 1);
				timer_settime(cons->timer, 50);
			}
			// 打开光标
			if (i == 2) {
				cons->cur_c = COL8_FFFFFF;
			}
			// 关闭光标
			else if (i == 3) {
				cons->cur_c = -1;
			}
			// 键盘数据
			else if (256 <= i) {
				reg[7] = i - 256;
				return 0;
			}
		}
	}
	// 获取定时器api
	// EDX = 16
	// EAX = 定时器句柄（由操作系统返回）
	else if (edx == 16){
		reg[7] = (int)timer_alloc();
		((struct TIMER *)reg[7])->flags2 = 1;
	}
	// 设置定时器发送的数据
	// EDX = 17
	// EBX = 定时器句柄
	// EAX = 数据
	else if (edx == 17) {
		timer_init((struct TIMER *)ebx, &task->fifo, eax + 256);
	}
	// 设置定时器时间
	// EDX = 18
	// EBX = 定时器句柄
	// EAX = 时间
	else if (edx == 18) {
		timer_settime((struct TIMER *)ebx, eax);
	}
	// 释放定时器
	// EDX = 19
	// EBX = 定时器句柄
	else if (edx == 19) {
		timer_free((struct TIMER *)ebx);
	}
	// 蜂鸣器发声
	// EDX = 20
	else if (edx == 20) {
		// 频率为零关闭蜂鸣器
		if (eax == 0) {
			int i = io_in8(0x61);
			io_out8(0x61, i & 0x0d);
		}
		else {
			int i = 1193180000 / eax;
			io_out8(0x43, 0xb6);
			io_out8(0x42, i & 0xff);
			io_out8(0x42, i >> 8);
			i = io_in8(0x61);
			io_out8(0x61, (i | 0x03) & 0x0f);
		}
	}
	return 0;
}

/**
 * 一般异常处理程序
 * @param esp		程序要还原的esp地址
 * @return			是否执行成功
 */
int *inthandler0d(int *esp) {
	struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
	struct TASK *task = task_now();
	char s[30];
	cons_putstr(cons, "\nINT 0D:\n General Protected EXception.\n");
	sprintf(s, "EIP = %08x\n", esp[11]);
	cons_putstr(cons, s);
	return &(task->tss.esp0);
}

/**
 * 栈溢出处理程序
 * @param esp		程序要还原的esp地址
 * @return			是否执行成功
 */
int *inthandler0c(int *esp) {
	struct CONSOLE *cons = (struct CONSOLE *)*((int *)0xfec);
	struct TASK *task = task_now();
	char s[30];
	cons_putstr(cons, "\nINT 0C:\n Stack EXception.\n");
	/*
	 * esp[0]:EDI
	 * esp[1]:ESI
	 * esp[2]:EBP
	 * esp[3]:ESP
	 * esp[4]:EBX
	 * esp[5]:EDX
	 * esp[6]:ECX
	 * esp[7]:EAX
	 * esp[8]:DS
	 * esp[9]:ES
	 * esp[10]:错误编号
	 * esp[11]:EIP
	 * esp[12]:CS
	 * esp[13]:EFLAGS
	 * esp[14]:应用程序ESP
	 * esp[15]:应用程序SS
	 */
	sprintf(s, "EIP = %08x\n", esp[11]);
	cons_putstr(cons, s);
	return &(task->tss.esp0);
}

/**
 * 画直线api
 * @param sht		窗口句柄
 * @param x0		起始x坐标
 * @param y0		起始y坐标
 * @param x1		结束x坐标
 * @param y1		结束y坐标
 * @param col		颜色
 */
void hrb_api_linewin(struct SHEET *sht, int x0, int y0, int x1, int y1, int col) {

	int i, x, y;
	// 需要绘制点的个数
	int len;
	// x方向上每隔dx个像素加一，y方向上每隔dy个像素加一
	int dx, dy;

	// 计算两点之间x,y的距离
	dx = x1 - x0;
	dy = y1 - y0;
	// 扩大2^10倍，避免浮点数运算
	x = x0 << 10;
	y = y0 << 10;
	if (dx < 0) {
		dx = -dx;
	}
	if (dy < 0) {
		dy = -dy;
	}

	// 比较两点之间x的差值与y的差值，将变化较大的作为点个数
	if (dx >= dy) {
		// 最后一个点也要显示
		len = dx + 1;
		// 确定dx
		if (x0 > x1) {
			// 由于扩大10倍参与运算，1024实际上就是移动1一个像素
			dx = -1024;
		}
		else {
			dx = 1024;
		}

		// 确定dy
		if (y0<= y1) {
			dy = ((y1 - y0 + 1) << 10) / len;
		}
		else {
			dy = ((y1 - y0 - 1) << 10) / len;
		}
	}
	else {
		len = dy + 1;
		if (y0 > y1) {
			dy = -1024;
		}
		else {
			dy = 1024;
		}
		if (x0 <= x1) {
			dx = ((x1 - x0 + 1) << 10) / len;
		}
		else {
			dx = ((x1 - x0 - 1) << 10) / len;
		}
	}

	// 绘制直线
	for (i = 0; i < len; i++) {
		// 相当于y * bxsize(窗口宽度) + x
		sht->buf[(y >> 10) * sht->bxsize + (x >> 10)] = col;
		x += dx;
		y += dy;
	}
	return;
}
