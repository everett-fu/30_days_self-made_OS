/*
 * Filename: fifo.c
 * Author: everett.fu
 * Created: 24-7-20
 * Last Modified: 24-7-20
 * Description:
 *
 * Functions:
 *
 * Usage:
 */

#include "bootpack.h"

#define FLAGS_OVERRUN 0x0001

/**
 * 初始化FIFO缓冲区
 * @param fifo		FIFO缓冲区
 * @param size		缓冲区大小
 * @param buf		缓冲区地址
 */
void fifo8_init(struct FIFO8 *fifo, int size, unsigned char *buf) {
	fifo->size = size;
	fifo->buf = buf;
	fifo->free = size;                // 缓冲区大小
	fifo->flags = 0;
	fifo->next_w = 0;                // 下一个写入位置
	fifo->next_r = 0;                // 下一个读取位置
	return;
}

/**
 * 从FIFO缓冲区中写入一个字节
 * @param fifo		FIFO缓冲区
 * @param data		写入的数据
 * @return 			缓冲区已满
 */
int fifo8_put(struct FIFO8 *fifo, unsigned char data) {
	// 缓冲区已满
	if (fifo->free == 0) {
		fifo->flags |= FLAGS_OVERRUN;
		return -1;
	}
	fifo->buf[fifo->next_w] = data;
	fifo->next_w++;
	// 如果写入位置到达缓冲区末尾，则回到开头
	if (fifo->next_w == fifo->size) {
		fifo->next_w = 0;
	}
	fifo->free--;
	return 0;
}

/**
 * 从FIFO缓冲区中读取一个字节
 * @param fifo		FIFO缓冲区
 * @return 			读取的数据或者-1
 */
int fifo8_get(struct FIFO8 *fifo) {
	int data;
	// 缓冲区为空
	if (fifo->free == fifo->size) {
		return -1;
	}
	data = fifo->buf[fifo->next_r];
	fifo->next_r++;
	// 如果读取位置到达缓冲区末尾，则回到开头
	if (fifo->next_r == fifo->size) {
		fifo->next_r = 0;
	}
	fifo->free++;
	return data;
}

/**
 * 获取FIFO缓冲区的状态
 * @param fifo		FIFO缓冲区
 * @return			缓冲区内的数量
 */
int fifo8_status(struct FIFO8 *fifo) {
	return fifo->size - fifo->free;
}
