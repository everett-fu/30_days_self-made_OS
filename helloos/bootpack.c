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

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(void);
void init_keyboard(void);

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;

	// 初始化GDT,IDT
	init_gdtidt();

	// 初始化PIC
	init_pic();

	// 将中断标志设置为1
	io_sti();

	// 初始化FIFO缓冲区
	fifo8_init(&keyfifo, 32, keybuf);
	fifo8_init(&mousefifo, 128, mousebuf);

	// 打开中断
	io_out8(PIC0_IMR, 0xf9);                // 允许键盘中断
	io_out8(PIC1_IMR, 0xef);                // 允许鼠标中断

	// 初始化键盘
	init_keyboard();

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
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	unsigned char mouse_dbuf[3], mouse_phase;
	// 激活鼠标
	enable_mouse();
	mouse_phase = 0;

	for (;;) {
		io_cli();
		// 判断是否有键盘输入
		// 如果没有键盘输入，则进入休眠状态
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		} else {
			// 如果有键盘输入，则显示键盘输入
			if (fifo8_status(&keyfifo) != 0) {
				i = fifo8_get(&keyfifo);
				io_sti();
				sprintf(s, "%02X", i);
				boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 16, 15, 31);
				putfonts8_asc(binfo->vram, binfo->scrnx, 0, 16, COL8_FFFFFF, s);
			}
			// 如果有鼠标输入，则显示鼠标输入
			else if (fifo8_status(&mousefifo) != 0) {
				i = fifo8_get(&mousefifo);
				io_sti();
				if (mouse_phase == 0) {
					if (i == 0xfa) {
						mouse_phase = 1;
					}
				} else if (mouse_phase == 1) {
					mouse_dbuf[0] = i;
					mouse_phase = 2;
				} else if (mouse_phase == 2) {
					mouse_dbuf[1] = i;
					mouse_phase = 3;
				} else if (mouse_phase == 3) {
					mouse_dbuf[2] = i;
					mouse_phase = 1;

					sprintf(s, "%02x %02x %02x", mouse_dbuf[0], mouse_dbuf[1], mouse_dbuf[2]);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 8 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);
				}
			}
		}
	}
}

#define PORT_KEYDAT 0x0060
#define PORT_KEYSTA 0x0064
#define PORT_KEYCMD 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

/**
 * 等待键盘控制电路准备完毕
 */
void wait_KBC_sendready(void) {
	// 等待键盘控制电路准备完毕
	for (;;) {
		if ((io_in8(PORT_KEYSTA) & KEYSTA_SEND_NOTREADY) == 0) {
			break;
		}
	}
	return;
}

/**
 * 初始化键盘
 */
void init_keyboard(void) {
	// 初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

/**
 * 激活鼠标
 */
void enable_mouse(void) {
	// 激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	return;
}
