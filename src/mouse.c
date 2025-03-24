/*
 * Filename: mouse.c
 * Author: everett.fu
 * Created: 24-7-30
 * Last Modified: 24-7-30
 * Description:
 * 这个文件包含了鼠标中断与鼠标初始化的实现。
 *
 * Functions:
 * - inthandler2c: 处理鼠标中断
 * - enable_mouse: 激活鼠标
 * - mouse_decode: 一次显示三个鼠标符
 *
 * Usage:
 */

#include "bootpack.h"

// 当发送d4以后，下一个数据会自动发送给鼠标
// 控制键盘将数据发送给鼠标的命令
#define KEYCMD_SENDTO_MOUSE 0xd4
// 开始鼠标电路指令
#define MOUSECMD_ENABLE 0xf4

struct FIFO32 *mousefifo;
int mousedata0;

/**
 * 处理鼠标中断
 * @param esp		中断栈指针
 */
void inthandler2c(int *esp) {
	int data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	fifo32_put(mousefifo, data + mousedata0);
	return;
}

/**
 * 激活鼠标
 * @param fifo		缓冲区
 * @param data0		数据
 * @param mdec		鼠标结构体
 */
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec) {
	mousefifo = fifo;
	mousedata0 = data0;
	// 激活鼠标
	wait_KBC_sendready();
	// 向键盘控制电路发送命令0xd4，下一个数据会自动发送给鼠标
	io_out8(PORT_KEYCMD, KEYCMD_SENDTO_MOUSE);
	wait_KBC_sendready();
	// 将命令0xf4发送给鼠标
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
