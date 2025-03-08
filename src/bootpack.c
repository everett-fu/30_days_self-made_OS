/*
 * Filename: bootpack.c
 * Author: everett.fu
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个文件是操作系统的主文件，包含了操作系统的入口函数和一些初始化函数。
 *
 * Functions:
 * - HariMain: 主函数
 * - make_window8: 创建窗口
 * - putfonts8_asc_sht: 在图层上显示字符串
 * - make_textbox8: 创建文本框
 * - console_task: 创建终端窗口
 *
 * Usage:
 */
#include <stdio.h>
#include "bootpack.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act);
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l);
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c);
void console_task(struct SHEET *sheet);
void make_wtitle8(unsigned char *buf, int xsize, char *title, char act);

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	struct FIFO32 fifo;
	char s[40];
	int fifobuf[128];

	struct TIMER *timer;
	int mx, my, i;
	struct MOUSE_DEC mdec;
	unsigned int memtotal;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;

	int cursor_x, cursor_c;

	// 创建任务a,b
	struct TASK *task_a, *task_cons;

	// 标志位，用来判断是否按下tab，用来切换任务窗口
	int key_to = 0;
	// 标志位，用来判断shift是否按下
	int key_shift = 0;

	// 初始化GDT,IDT
	init_gdtidt();

	// 初始化PIC
	init_pic();

	// 初始化定时器
	init_pit();

	// 将中断标志设置为1，允许中断
	io_sti();

	// 初始化FIFO缓冲区
	fifo32_init(&fifo, 128, fifobuf, 0);

	// 申请定时器，并初始化与设置定时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	// 打开中断
	io_out8(PIC0_IMR, 0xf8);                // PIC1和键盘许可(11111000)
	io_out8(PIC1_IMR, 0xef);                // 鼠标许可(11101111)

	// 初始化键盘
	init_keyboard(&fifo, 256);
	// 激活鼠标
	enable_mouse(&fifo, 512, &mdec);
	// 初始化内存管理
	// 获取内存大小
	memtotal = memtest(0x00400000, 0xbfffffff);
	// 初始化内存管理
	memman_init(memman);
	memman_free(memman, 0x00001000,0x0009e000);
	memman_free(memman, 0x00400000,memtotal - 0x00400000);

	// 初始化任务
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	// 初始化调色板
	init_palette();

	// 初始化图层控制
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);

	// 背景图层
	// 创建背景图层
	sht_back = sheet_alloc(shtctl);
	// 分配背景图层缓冲区
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	// 设置背景图层大小和透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	// 将类Windows效果放置到背景图层之中
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	// 鼠标图层
	// 创建鼠标图层
	sht_mouse = sheet_alloc(shtctl);
	// 设置鼠标图层大小和透明色
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	// 将鼠标指针放置到鼠标图层之中
	init_mouse_cursor8(buf_mouse, 99);
	// 计算显示中间位置
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	// 窗口a图层
	// 创建窗口图层
	sht_win = sheet_alloc(shtctl);
	// 分配窗口图层缓冲区
	buf_win = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
	// 设置窗口图层大小和透明色
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	// 将窗口放置到窗口图层之中
	make_window8(buf_win, 144, 52, "editor", 1);
	// 将文本框放置到窗口图层中
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	// 光标位置
	cursor_x = 8;
	// 光标颜色
	cursor_c = COL8_FFFFFF;

	// 终端图层
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	// 为任务b的堆栈分配了64kb的内存，并计算出栈底的内存地址
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 8;
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	task_run(task_cons, 2, 2);

	// 背景色填充
	sheet_slide(sht_back, 0, 0);
	// 显示鼠标
	sheet_slide(sht_mouse, mx, my);
	// 显示窗口
	sheet_slide(sht_win, 8, 56);
	sheet_slide(sht_cons, 32, 4);
	// 设置背景图层高度
	sheet_updown(sht_back, 0);
	// 设置窗口图层高度
	sheet_updown(sht_win, 1);
	sheet_updown(sht_cons, 1);
	// 设置鼠标图层高度
	sheet_updown(sht_mouse, 10);

	// 显示鼠标坐标
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);

	// 显示内存信息
	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc_sht(sht_back, 0, 32, COL8_FFFFFF, COL8_008484, s, 40);

	// 系统主循环
	for (;;) {
		//sprintf(s, "%010d", timerctl.count);
		//putfonts8_asc_sht(sht_win, 40, 28, COL8_000000, COL8_C6C6C6, s, 10);

		// 屏蔽中断
		io_cli();
		// 判断是否有键盘输入，或者鼠标输入，或者定时器超时
		// 如果输入缓冲中没有任何的数据，则进入休眠状态
		if (fifo32_status(&fifo) == 0) {
			// 如果没有中断，自己休眠自己
			task_sleep(task_a);
			io_sti();
		}
		// 有中断
		else {
			i = fifo32_get(&fifo);
			io_sti();
			// 如果有键盘输入，则显示键盘输入
			if (i >=256 && i <= 511) {
				sprintf(s, "%02x", i - 256);
				putfonts8_asc_sht(sht_back, 0, 16, COL8_FFFFFF, COL8_008484, s, 2);
				// 一般字符
				if (i < 256 + 0x80 && keytable0[i - 256] != 0) {
					// shift没有被按下
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					}
					else {
						s[0] = keytable1[i - 256];
					}
					// 给任务a的数据
					if (key_to == 0) {
						if (cursor_x < 128) {
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_FFFFFF, COL8_008484, s, 1);
							cursor_x += 8;
						}
					}
					// 给任务b的数据
					else {
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				// 其他字符
				else {
					// 退格键
					if (i == 256 + 0x0e) {
						// 任务a
						if (key_to == 0) {
							if (cursor_x > 8) {
								// 把光标的位置变成背景颜色，再改上一个字符的颜色
								putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
								cursor_x -=8;
								boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
								sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
							}

						}
						// 任务b
						else {
							fifo32_put(&task_cons->fifo, 8 + 256);
						}
					}
					// TAB键
					else if (i == 256 + 0x0f) {
						if (key_to == 0) {
							key_to = 1;
							make_wtitle8(buf_win, sht_win->bxsize, "task_a", 0);
							make_wtitle8(buf_cons, sht_cons->bxsize, "console", 1);
						}
						else {
							key_to = 0;
							make_wtitle8(buf_win, sht_win->bxsize, "task_a", 1);
							make_wtitle8(buf_cons, sht_cons->bxsize, "console", 0);
						}
						sheet_refresh(sht_win, 0, 0, sht_win->bxsize, 21);
						sheet_refresh(sht_cons, 0, 0, sht_cons->bxsize, 21);
					}
					// 左Shift ON
					else if (i == 256 + 0x2a) {
						key_shift |= 1;
					}
					// 左Shift OFF
					else if (i == 256 + 0xaa) {
						key_shift &= -1;
					}
					// 右Shift ON
					else if (i == 256 + 0x36) {
						key_shift |= 2;
					}
					// 右Shift OFF
					else if (i == 256 + 0xb6) {
						key_shift &= -2;
					}
				}
			}
			// 如果有鼠标输入，则显示鼠标输入
			else if (i >= 512 && i <=767) {
				// 如果鼠标的数据接收完全
				if (mouse_decode(&mdec, i - 512) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
						sheet_slide(sht_win, mx - 80, my -8);
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					putfonts8_asc_sht(sht_back, 32, 16, COL8_FFFFFF, COL8_008484, s, 15);

					// 鼠标指针的移动
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					// 显示鼠标坐标
					sprintf(s, "(%3d, %3d)", mx, my);
					putfonts8_asc_sht(sht_back, 0, 0, COL8_FFFFFF, COL8_008484, s, 10);
					// 显示鼠标
					sheet_slide(sht_mouse, mx, my);
				}
			}
			// 光标寄存器
			else if (i <= 1) {
				if (i == 1) {
					timer_init(timer, &fifo, 0);
					cursor_c = COL8_000000;
				}
				else if (i == 0) {
					timer_init(timer, &fifo, 1);
					cursor_c = COL8_FFFFFF;
				}
				timer_settime(timer, 50);
				boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
		}
	}
}

/**
 * 创建窗口
 * @param buf		缓冲区
 * @param xsize		窗口的宽度
 * @param ysize		窗口的高度
 * @param title		窗口的标题
 * @param act		窗口颜色，如果为1则是彩色，如果为0则是黑色
 */
void
make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act) {
	// 上阴影
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	// 左阴影
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize -  2);
	// 右阴影
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	// 窗口主体
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	// 下阴影
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	// 窗口标题与窗口关闭按钮
	make_wtitle8(buf, xsize, title, act);
	return;
}
/**
 * 窗口标题与窗口关闭按钮
 * @param buf		缓冲区
 * @param xsize		窗口的宽度
 * @param title		窗口的标题
 * @param act		窗口颜色，如果为1则是彩色，如果为0则是黑色
 */

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act) {
	static char closebtn[14][16]= {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};

	// 判断窗口标题颜色
	int x, y;
	char c, tc, tbc;
	if (act == 1) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	}
	else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize,24, 4, tc, title );
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			}
			else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}

}

/**
 * 绘制字符串
 * @param sht		图层
 * @param x			起始位置x
 * @param y			起始位置y
 * @param c			颜色
 * @param b			背景色
 * @param s			输入的字符串
 * @param l			字符串长度
 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l) {
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

/**
 * 绘制编辑器白色文本框
 * @param sht		图层
 * @param x0		起始位置x
 * @param y0		起始位置y
 * @param sx		文本框长度
 * @param sy		文本框宽度
 * @parma c			文本框颜色
 */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

/**
 * 任务b
 * @param sht_back		要显示的图层地址
*/
void console_task(struct SHEET *sheet){
	// 缓冲区数据
	int fifobuf[128];
	// 界面刷新定时器
	struct TIMER *timer;
	// 获取当前任务的地址
	struct TASK *task = task_now();
	// 临时变量，字符位置，字符颜色
	int i, cursor_x = 16, cursor_c = COL8_000000;
	// 临时变量，用于存储字符
	char s[2];

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		}
		else {
			i = fifo32_get(&task->fifo);
			io_sti();
			if (i <= 1) {
				if (i == 1) {
					timer_init(timer, &task->fifo, 0);
					cursor_c = COL8_FFFFFF;
				}
				else {
					timer_init(timer, &task->fifo, 1);
					cursor_c = COL8_000000;
				}
				timer_settime(timer, 50);
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
			}
			// 如果有键盘输入，则显示键盘输入
			if (i >=256 && i <= 511) {
				// 退格符
				if (i == 8 + 256) {
					if (cursor_x > 16) {
						// 把光标的位置变成背景颜色，再改上一个字符的颜色
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -=8;
						boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
						sheet_refresh(sheet, cursor_x, 28, cursor_x + 8, 44);
					}
				}
				// 一般字符
				else {
					if ( cursor_x < 240) {
						s[0] = i - 256;
						s[1] = 0;
						putfonts8_asc_sht(sheet, cursor_x, 28, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
		}
	}
}
