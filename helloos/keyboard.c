/*
 * Filename: keyboard.c
 * Author: everett.fu
 * Created: 24-7-30
 * Last Modified: 24-7-30
 * Description:
 * 这个文件包含了键盘中断与键盘初始化的实现。
 *
 * Functions:
 * - inthandler21: 处理键盘中断
 * - wait_KBC_sendready: 等待键盘控制电路准备完毕
 * - init_keyboard: 初始化键盘
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
 * @param fifo		缓冲区
 * @param data0		数据
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
