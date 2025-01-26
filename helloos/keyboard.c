/*
 * Filename: keyboard.c
 * Author: everett.fu
 * Created: 24-7-30
 * Last Modified: 24-7-30
 * Description:
 *   This file contains the implementation of keyboard. The program
 *   demonstrates basic functionality and serves as an example.
 *
 * Functions:
 *   - main: The entry point of the program.
 *   - ${Function1}: Description of the function.
 *   - ${Function2}: Description of the function.
 *
 * Usage:
 */
#include "bootpack.h"

#define PORT_KEYSTA 0x0064
#define KEYSTA_SEND_NOTREADY 0x02
#define KEYCMD_WRITE_MODE 0x60
#define KBC_MODE 0x47

struct FIFO32 *keyfifo;
int keydata0;

/**
 * 处理键盘中断
 * @param esp
 */
void inthandler21(int *esp) {
	int data;
	io_out8(PIC0_OCW2, 0x61);
	data = io_in8(PORT_KEYDAT);
	fifo32_put(keyfifo, data + keydata0);
	return;
}

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
void init_keyboard(struct FIFO32 *fifo, int data0) {
	keyfifo = fifo;
	keydata0 = data0;
	// 初始化键盘控制电路
	wait_KBC_sendready();
	io_out8(PORT_KEYCMD, KEYCMD_WRITE_MODE);
	wait_KBC_sendready();
	io_out8(PORT_KEYDAT, KBC_MODE);
	return;
}
