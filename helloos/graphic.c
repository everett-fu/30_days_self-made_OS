/*
 * Filename: graphic.c
 * Author: everett.fu
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个函数包含了一些绘图函数，用于绘制图形。
 *
 * Functions:
 * - init_palette: 初始化调色板
 * - set_palette: 设置调色板
 * - boxfill8: 绘制纯色矩形
 * - init_screen8: 绘制类Windows效果
 * - putfont8: 绘制字符
 * - putfonts8_asc: 绘制字符串
 * - init_mouse_cursor8: 初始化鼠标指针
 * - putblock8_8: 绘制鼠标指针
 *
 * Usage:
 */

#include "bootpack.h"

/**
 * 初始化调色板
 */
void init_palette(void) {
	static unsigned char table_rgb[16 * 3] = {
			0x00, 0x00, 0x00,        // 黑
			0xff, 0x00, 0x00,        // 亮红
			0x00, 0xff, 0x00,        // 亮緑
			0xff, 0xff, 0x00,        // 亮黄
			0x00, 0x00, 0xff,    // 亮蓝
			0xff, 0x00, 0xff,    // 亮紫
			0x00, 0xff, 0xff,    // 浅亮蓝
			0xff, 0xff, 0xff,    // 白
			0xc6, 0xc6, 0xc6,    // 亮灰
			0x84, 0x00, 0x00,    // 暗红
			0x00, 0x84, 0x00,    // 暗緑
			0x84, 0x84, 0x00,    // 暗黄
			0x00, 0x00, 0x84,    // 暗青
			0x84, 0x00, 0x84,    // 暗紫
			0x00, 0x84, 0x84,    // 浅暗蓝
			0x84, 0x84, 0x84        // 暗灰
	};
	set_palette(0, 15, table_rgb);
	return;
}

/**
 * 设置调色板
 * @param start		开始
 * @param end		结束
 * @param rgb		rgb
 */
void set_palette(int start, int end, unsigned char *rgb) {
	int i, eflags;
	// 中断标志
	eflags = io_load_eflags();

	// 屏蔽中断
	io_cli();
	io_out8(0x03c8, start);
	// 导入0到15号颜色
	for (i = start; i <= end; i++) {
		io_out8(0x03c9, rgb[0] / 4);
		io_out8(0x03c9, rgb[1] / 4);
		io_out8(0x03c9, rgb[2] / 4);
		rgb += 3;
	}
	// 恢复中断
	io_store_eflags(eflags);
	return;
}

/**
 * 绘制纯色矩形
 * @param vram		显存地址
 * @param xsize		屏幕宽度
 * @param c			颜色
 * @param x0		开始的x0坐标
 * @param y0		开始的y0坐标
 * @param x1		结束的x1坐标
 * @param y1		结束的y1坐标
 */
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1) {
	int x, y;
	for (y = y0; y <= y1; y++) {
		for (x = x0; x <= x1; x++) {
			vram[y * xsize + x] = c;
		}
	}
	return;
}

/**
 * 绘制类Windows效果
 * @param vram		显存地址
 * @param xsize		宽
 * @param ysize		高
 */
void init_screen8(unsigned char *vram, int xsize, int ysize) {
	// 绘制背景
	boxfill8(vram, xsize, COL8_008484, 0, 0, xsize - 1, ysize - 29);
	boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 28, xsize - 1, ysize - 28);
	boxfill8(vram, xsize, COL8_FFFFFF, 0, ysize - 27, xsize - 1, ysize - 27);
	boxfill8(vram, xsize, COL8_C6C6C6, 0, ysize - 26, xsize - 1, ysize - 1);

	//
	boxfill8(vram, xsize, COL8_FFFFFF, 3, ysize - 24, 59, ysize - 24);
	boxfill8(vram, xsize, COL8_FFFFFF, 2, ysize - 24, 2, ysize - 4);
	boxfill8(vram, xsize, COL8_848484, 3, ysize - 4, 59, ysize - 4);
	boxfill8(vram, xsize, COL8_848484, 59, ysize - 23, 59, ysize - 5);
	boxfill8(vram, xsize, COL8_000000, 2, ysize - 3, 59, ysize - 3);
	boxfill8(vram, xsize, COL8_000000, 60, ysize - 24, 60, ysize - 3);

	//
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 24, xsize - 4, ysize - 24);
	boxfill8(vram, xsize, COL8_848484, xsize - 47, ysize - 23, xsize - 47, ysize - 4);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 47, ysize - 3, xsize - 4, ysize - 3);
	boxfill8(vram, xsize, COL8_FFFFFF, xsize - 3, ysize - 24, xsize - 3, ysize - 3);
	return;
}

/**
 * 绘制字符
 * @param vram		显存地址
 * @param xsize		屏幕的宽度
 * @param x			起始位置x
 * @param y			起始位置y
 * @param c			颜色
 * @param font		字体
 */
void putfont8(unsigned char *vram, int xsize, int x, int y, char c, char *font) {
	int i;
	unsigned char *p;
	// 判断字符的每一行
	for (i = 0; i < 16; i++) {
		p = vram + (y + i) * xsize + x;
		// 判断字符的每一位是否为1，如果是则显示颜色
		char num = font[i];
		int j;
		for (j = 0; j < 8; j++, num = num >> 1) {
			if ((num & 1) == 1) {
				p[8 - j] = c;
			}
		}
	}
	return;
}

/**
 * 绘制字符串
 * @param vram		显存地址
 * @param xsize		屏幕的宽度
 * @param x			起始位置x
 * @param y			起始位置y
 * @param c			颜色
 * @param s			输入的字符串
 */
void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, unsigned char *s) {
	// 导入字符集
	extern char hankaku[4096];
	// 循环字符串，一直到字符串结束
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
	return;
}

/**
 * 初始化鼠标指针
 * @param mouse		鼠标指针地址
 * @param bc		背景颜色
 */
void init_mouse_cursor8(char *mouse, char bc) {
	static char cursor[16][16] = {
			"**************..",
			"*OOOOOOOOOOO*...",
			"*OOOOOOOOOO*....",
			"*OOOOOOOOO*.....",
			"*OOOOOOOO*......",
			"*OOOOOOO*.......",
			"*OOOOOOO*.......",
			"*OOOOOOOO*......",
			"*OOOO**OOO*.....",
			"*OOO*..*OOO*....",
			"*OO*....*OOO*...",
			"*O*......*OOO*..",
			"**........*OOO*.",
			"*..........*OOO*",
			"............*OO*",
			".............***"
	};

	int x, y;

	for (y = 0; y < 16; y++) {
		for (x = 0; x < 16; x++) {
			if (cursor[y][x] == '*') {
				mouse[y * 16 + x] = COL8_000000;
			}
			else if (cursor[y][x] == 'O') {
				mouse[y * 16 + x] = COL8_FFFFFF;
			}
			else {
				mouse[y * 16 + x] = bc;
			}
		}
	}
	return;
}

/**
 * 绘制鼠标指针
 * @param vram		显存地址
 * @param vxsize	屏幕的宽度
 * @param pxsize	鼠标指针的宽度
 * @param pysize	鼠标指针的高度
 * @param px0		鼠标指针的x坐标
 * @param py0		鼠标指针的y坐标
 * @param buf		鼠标指针的颜色
 * @param bxsize	鼠标指针的宽度
 */
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize) {
	int x, y;
	for (y = 0; y < pysize; y++) {
		for (x = 0; x < pxsize; x++) {
			vram[(py0 + y) * vxsize + (px0 + x)] = buf[y * bxsize + x];
		}
	}
	return;
}
