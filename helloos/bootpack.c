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
 *
 * Usage:
 */
#include <stdio.h>
#include "bootpack.h"

void make_window8(unsigned char *buf, int xsize, int ysize, char *title);

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;
	unsigned int memtotal, count = 0;
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	struct SHTCTL *shtctl;
	struct SHEET *sht_back, *sht_mouse, *sht_win;
	unsigned char *buf_back, buf_mouse[256], *buf_win;

	// 初始化GDT,IDT
	init_gdtidt();

	// 初始化PIC
	init_pic();

	// 将中断标志设置为1，允许中断
	io_sti();

	// 初始化FIFO缓冲区
	// 键盘缓冲区
	fifo8_init(&keyfifo, 32, keybuf);
	// 鼠标缓冲区
	fifo8_init(&mousefifo, 128, mousebuf);

	// 初始化定时器
	init_pit();

	// 打开中断
	io_out8(PIC0_IMR, 0xf8);                // PIC1和键盘许可(11111000)
	io_out8(PIC1_IMR, 0xef);                // 鼠标许可(11101111)

	// 初始化键盘
	init_keyboard();
	// 激活鼠标
	enable_mouse(&mdec);
	// 初始化内存管理
	// 获取内存大小
	memtotal = memtest(0x00400000, 0xbfffffff);
	// 初始化内存管理
	memman_init(memman);
	memman_free(memman, 0x00001000,0x0009e000);
	memman_free(memman, 0x00400000,memtotal - 0x00400000);


	// 初始化调色板
	init_palette();

	// 初始化图层控制
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	// 创建背景图层
	sht_back = sheet_alloc(shtctl);
	// 创建鼠标图层
	sht_mouse = sheet_alloc(shtctl);
	// 创建窗口图层
	sht_win = sheet_alloc(shtctl);
	// 分配背景图层缓冲区
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	// 分配窗口图层缓冲区
	buf_win = (unsigned char *) memman_alloc_4k(memman, 160 * 52);
	// 设置背景图层大小和透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	// 设置鼠标图层大小和透明色
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	// 设置窗口图层大小和透明色
	sheet_setbuf(sht_win, buf_win, 160, 52, -1);
	// 将类Windows效果放置到背景图层之中
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);
	// 将鼠标指针放置到鼠标图层之中
	init_mouse_cursor8(buf_mouse, 99);
	// 将窗口放置到窗口图层之中
	make_window8(buf_win, 160, 52, "counter");
	// 背景色填充
	sheet_slide(sht_back, 0, 0);

	// 计算显示中间位置
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	// 显示鼠标
	sheet_slide(sht_mouse, mx, my);
	// 显示窗口
	sheet_slide(sht_win, 80, 72);
	// 设置背景图层高度
	sheet_updown(sht_back, 0);
	// 设置鼠标图层高度
	sheet_updown(sht_mouse, 2);
	// 设置窗口图层高度
	sheet_updown(sht_win, 1);

	// 显示鼠标坐标
	sprintf(s, "(%3d, %3d)", mx, my);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	sprintf(s, "memory %dMB free : %dKB", memtotal / (1024 * 1024), memman_total(memman) / 1024);
	putfonts8_asc(buf_back, binfo->scrnx, 0, 32, COL8_FFFFFF, s);

	// 刷新所有图层
	sheet_refresh(sht_back, 0, 0, binfo->scrnx, 48);

	// 系统主循环
	for (;;) {
		count++;
		sprintf(s, "%010d", count);
		boxfill8(buf_win, 160, COL8_C6C6C6, 40, 28, 119, 43);
		putfonts8_asc(buf_win, 160, 40, 28, COL8_000000, s);
		sheet_refresh(sht_win, 40, 28, 120, 44);
		// 屏蔽中断
		io_cli();
		// 判断是否有键盘输入，或者鼠标输入
		// 如果没有键盘输入或者鼠标输入，则进入休眠状态
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_sti();
//			io_stihlt();
		}
		else {
			// 如果有键盘输入，则显示键盘输入
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(buf_back, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
				sheet_refresh(sht_back, 0, 16, 16, 32);
			}
				// 如果有鼠标输入，则显示鼠标输入
			else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				// 如果鼠标的数据接收完全
				if (mouse_decode(&mdec, i) != 0) {
					sprintf(s, "[lcr %4d %4d]", mdec.x, mdec.y);
					if ((mdec.btn & 0x01) != 0) {
						s[1] = 'L';
					}
					if ((mdec.btn & 0x02) != 0) {
						s[3] = 'R';
					}
					if ((mdec.btn & 0x04) != 0) {
						s[2] = 'C';
					}
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(buf_back, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
					sheet_refresh(sht_back, 32, 16, 32 + 15 * 8, 32);

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
					boxfill8(buf_back, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					putfonts8_asc(buf_back, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					// 显示鼠标
					sheet_refresh(sht_back, 0, 0, 80, 16);
					sheet_slide(sht_mouse, mx, my);
				}
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
 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title) {
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
	int x, y;
	char c;
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize -  2);
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	boxfill8(buf, xsize, COL8_000084, 3, 3, xsize - 4, 20);
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	putfonts8_asc(buf, xsize,24, 4, COL8_FFFFFF, title );
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			}
			else if (c == '$') {
				c = COL8_848484;
			}
			else if (c == 'Q') {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
	return;
}