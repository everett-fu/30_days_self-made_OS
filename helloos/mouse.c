/*
 * Filename: mouse.c
 * Author: everett.fu
 * Created: 24-7-30
 * Last Modified: 24-7-30
 * Description:
 *   This file contains the implementation of mouse. The program
 *   demonstrates basic functionality and serves as an example.
 *
 * Functions:
 *   - main: The entry point of the program.
 *   - ${Function1}: Description of the function.
 *   - ${Function2}: Description of the function.
 *
 * Usage:
 *   To compile: gcc -o mouse mouse.c
 *   To run: ./ mouse
 */

#include "bootpack.h"

#define KEYCMD_SENDTO_MOUSE 0xd4
#define MOUSECMD_ENABLE 0xf4

/**
 * 处理鼠标中断
 * @param esp
 */
struct FIFO8 mousefifo;
void inthandler2c(int *esp) {
	unsigned char data;
	io_out8(PIC1_OCW2, 0x64);
	io_out8(PIC0_OCW2, 0x62);
	data = io_in8(PORT_KEYDAT);
	fifo8_put(&mousefifo, data);
	return;
}

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