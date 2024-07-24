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

struct MOUSE_DEC {
	// 鼠标符，鼠标状态
	unsigned char buf[3], phase;
	// x: 鼠标移动x坐标, y: 鼠标移动y坐标, btn: 按钮状态
	int x, y, btn;
};

extern struct FIFO8 keyfifo, mousefifo;
void enable_mouse(struct MOUSE_DEC *mdec);
void init_keyboard(void);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	char s[40], mcursor[256], keybuf[32], mousebuf[128];
	int mx, my, i;
	struct MOUSE_DEC mdec;

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
	// 显示鼠标
	putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
	// 显示鼠标坐标
	sprintf(s, "(%d, %d)", mx, my);
	putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);

	// 激活鼠标
	enable_mouse(&mdec);

	// 系统主循环
	for (;;) {
		// 屏蔽中断
		io_cli();
		// 判断是否有键盘输入，或者鼠标输入
		// 如果没有键盘输入或者鼠标输入，则进入休眠状态
		if (fifo8_status(&keyfifo) + fifo8_status(&mousefifo) == 0) {
			io_stihlt();
		}
		else {
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
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 32, 16, 32 + 15 * 8 - 1, 31);
					putfonts8_asc(binfo->vram, binfo->scrnx, 32, 16, COL8_FFFFFF, s);

					// 鼠标移动
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, mx, my, mx + 15, my + 15);
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 16) {
						mx = binfo->scrnx - 16;
					}
					if (my > binfo->scrnx - 16) {
						my = binfo->scrnx - 16;
					}
					// 显示鼠标坐标
					sprintf(s, "(%3d, %3d)", mx, my);
					boxfill8(binfo->vram, binfo->scrnx, COL8_008484, 0, 0, 79, 15);
					putfonts8_asc(binfo->vram, binfo->scrnx, 0, 0, COL8_FFFFFF, s);
					// 显示鼠标
					putblock8_8(binfo->vram, binfo->scrnx, 16, 16, mx, my, mcursor, 16);
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
void enable_mouse(struct MOUSE_DEC *mdec) {
	// 激活鼠标
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, MOUSECMD_ENABLE);
	mdec->phase = 0;
	return;
}

/**
 * 一次显示三个鼠标符
 * @param mdec		鼠标结构体
 * @param data		鼠标数据
 * @return			返回1表示成功，返回-1表示失败，返回0表示还没有接收完全
 */
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data) {
	if (mdec->phase == 0) {
		if (data == 0xfa) {
			mdec->phase = 1;
		}
		return 0;
	}
	else if (mdec->phase == 1) {
		if ((data & 0xc8) == 0x08) {
			mdec->buf[0] = data;
			mdec->phase = 2;
		}
		return 0;
	}
	else if (mdec->phase == 2) {
		mdec->buf[1] = data;
		mdec->phase = 3;
		return 0;
	}
	else if (mdec->phase == 3) {
		mdec->buf[2] = data;
		mdec->phase = 1;
		// 取出低三位数据
		mdec->btn = mdec->buf[0] & 0x07;
		mdec->x = mdec->buf[1];
		mdec->y = mdec->buf[2];

		if ((mdec->buf[0] & 0x10) != 0) {
			mdec->x |= 0xffffff00;
		}
		if ((mdec->buf[0] & 0x20) != 0) {
			mdec->y |= 0xffffff00;
		}
		mdec->y = -mdec->y;
		return 1;
	}
	return -1;
}
