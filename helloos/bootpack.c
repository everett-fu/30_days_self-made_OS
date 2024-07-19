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

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;
	char s[40], mcursor[256];
	int mx, my;

	// 初始化GDT,IDT
	init_gdtidt();

	// 初始化PIC
	init_pic();

	// 将中断标志设置为1
	io_sti();

	// 初始化调色板
	init_palette();

	// 显示类Windows效果
	init_screen8(binfo->vram, binfo->scrnx, binfo->scrny);

	// 计算显示中间位置
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;
	// 初始化鼠标指针
	init_mouse_cursor8(mcursor, COL8_008484);
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);

	// 显示字符串
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 8, COL8_FFFFFF, "Welcome to my OS.");

	// 显示鼠标坐标
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 8, 24, COL8_FFFFFF, s);

	io_out8(PIC0_IMR, 0xf9);
	io_out8(PIC1_IMR, 0xef);

	for (;;) {
		io_hlt();
	}
}