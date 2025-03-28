/*
 * Filename: int.c
 * Author: everett
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个文件包含了一些关于中断的函数。
 *
 * Functions:
 * - init_pic: 初始化PIC
 * - inthandler27: 处理PIC0的不完全中断
 *
 * Usage:
 */

#include <stdio.h>
#include "bootpack.h"

/**
 * 初始化PIC
 */
void init_pic(void) {
	io_out8(PIC0_IMR, 0xff);                    // 禁止主PIC所有中断
	io_out8(PIC1_IMR, 0xff);                    // 禁止从PIC所有中断

	io_out8(PIC0_ICW1, 0x11);                    // 边沿触发模式
	io_out8(PIC0_ICW2, 0x20);                    // IRQ0-7由INT20-27接收
	io_out8(PIC0_ICW3, 1 << 2);                    // 设定PIC1由IRQ2连接
	io_out8(PIC0_ICW4, 0x01);                    // 无缓冲区模式

	io_out8(PIC1_ICW1, 0x11);                    // 边沿触发模式
	io_out8(PIC1_ICW2, 0x28);                    // IRQ8-15由INT28-2f接收
	io_out8(PIC1_ICW3, 2);                        // 设定PIC1由IRQ2连接
	io_out8(PIC1_ICW4, 0x01);                    // 无缓冲区模式

	io_out8(PIC0_IMR, 0xfb);                    // 11111011 PIC1以外全部禁止
	io_out8(PIC1_IMR, 0xff);                    // 11111111 禁止所有中断

	return;
}

/**
 * 处理PIC0的不完全中断
 * @param esp		中断栈指针
 */
void inthandler27(int *esp)
/*
 * 处理PIC0的不完全中断
 * 在Athlon64X2等机器中，由于芯片组的原因，PIC初始化时会发生这种中断
 * 这个中断处理函数不做任何事情，直接结束
 * 为什么不需要做任何事情？
 * 这个中断是由PIC初始化时的电气噪声引起的，所以不需要认真处理它。
 */
{
	io_out8(PIC0_OCW2, 0x67); /* 向 PIC 发送 IRQ-07 接收完成的通知（见 7-1） */
	return;
}
