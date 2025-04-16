/*
 * Filename: window.c
 * Author: everett.fu
 * Created: 25-3-25
 * Last Modified: 25-3-25
 * Description:
 * 这个文件是窗口相关的文件，包含了窗口的创建与绘制函数。
 *
 *Functions:
 * - make_window8: 创建窗口
 * - make_wtitle8: 创建窗口标题
 * - putfonts8_asc_sht: 绘制字符串
 * - make_textbox8: 创建文本框
 * - make_textbox8_sht: 创建文本框
 *
 * Usage:
 */

#include "bootpack.h"

/**
 * 创建窗口
 * @param buf		缓冲区
 * @param xsize		窗口的宽度
 * @param ysize		窗口的高度
 * @param title		窗口的标题
 * @param act		窗口颜色，如果为1则是彩色，如果为0则是黑色
 */
void make_window8(unsigned char *buf, int xsize, int ysize, char *title, char act) {
	// 上阴影
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, xsize - 1, 0);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, xsize - 2, 1);
	// 左阴影
	boxfill8(buf, xsize, COL8_C6C6C6, 0, 0, 0, ysize - 1);
	boxfill8(buf, xsize, COL8_FFFFFF, 1, 1, 1, ysize -  2);
	// 右阴影
	boxfill8(buf, xsize, COL8_848484, xsize - 2, 1, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, xsize - 1, 0, xsize - 1, ysize - 1);
	// 窗口主体
	boxfill8(buf, xsize, COL8_C6C6C6, 2, 2, xsize - 3, ysize - 3);
	// 下阴影
	boxfill8(buf, xsize, COL8_848484, 1, ysize - 2, xsize - 2, ysize - 2);
	boxfill8(buf, xsize, COL8_000000, 0, ysize - 1, xsize - 1, ysize - 1);
	// 窗口标题与窗口关闭按钮
	make_wtitle8(buf, xsize, title, act);
	return;
}
/**
 * 窗口标题与窗口关闭按钮
 * @param buf		缓冲区
 * @param xsize		窗口的宽度
 * @param title		窗口的标题
 * @param act		窗口颜色，如果为1则是彩色，如果为0则是黑色
 */

void make_wtitle8(unsigned char *buf, int xsize, char *title, char act) {
	static char closebtn[14][16]= {
		"OOOOOOOOOOOOOOO@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQQQ@@QQQQQ$@",
		"OQQQQQ@@@@QQQQ$@",
		"OQQQQ@@QQ@@QQQ$@",
		"OQQQ@@QQQQ@@QQ$@",
		"OQQQQQQQQQQQQQ$@",
		"OQQQQQQQQQQQQQ$@",
		"O$$$$$$$$$$$$$$@",
		"@@@@@@@@@@@@@@@@"
	};

	// 判断窗口标题颜色
	int x, y;
	char c, tc, tbc;
	if (act == 1) {
		tc = COL8_FFFFFF;
		tbc = COL8_000084;
	}
	else {
		tc = COL8_C6C6C6;
		tbc = COL8_848484;
	}
	boxfill8(buf, xsize, tbc, 3, 3, xsize - 4, 20);
	putfonts8_asc(buf, xsize,24, 4, tc, title );
	// 显示关闭按钮
	for (y = 0; y < 14; y++) {
		for (x = 0; x < 16; x++) {
			c = closebtn[y][x];
			if (c == '@') {
				c = COL8_000000;
			}
			else if (c == '$') {
				c = COL8_848484;
			} else if (c == 'Q') {
				c = COL8_C6C6C6;
			} else {
				c = COL8_FFFFFF;
			}
			buf[(5 + y) * xsize + (xsize - 21 + x)] = c;
		}
	}
}

/**
 * 绘制字符串
 * @param sht		图层
 * @param x			起始位置x
 * @param y			起始位置y
 * @param c			颜色
 * @param b			背景色
 * @param s			输入的字符串
 * @param l			字符串长度
 */
void putfonts8_asc_sht(struct SHEET *sht, int x, int y, int c, int b, char *s, int l) {
	boxfill8(sht->buf, sht->bxsize, b, x, y, x + l * 8 - 1, y + 15);
	putfonts8_asc(sht->buf, sht->bxsize, x, y, c, s);
	sheet_refresh(sht, x, y, x + l * 8, y + 16);
	return;
}

/**
 * 绘制编辑器白色文本框
 * @param sht		图层
 * @param x0		起始位置x
 * @param y0		起始位置y
 * @param sx		文本框长度
 * @param sy		文本框宽度
 * @parma c			文本框颜色
 */
void make_textbox8(struct SHEET *sht, int x0, int y0, int sx, int sy, int c) {
	int x1 = x0 + sx, y1 = y0 + sy;
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 2, y0 - 3, x1 + 1, y0 - 3);
	boxfill8(sht->buf, sht->bxsize, COL8_848484, x0 - 3, y0 - 3, x0 - 3, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x0 - 3, y1 + 2, x1 + 1, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_FFFFFF, x1 + 2, y0 - 3, x1 + 2, y1 + 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 1, y0 - 2, x1 + 0, y0 - 2);
	boxfill8(sht->buf, sht->bxsize, COL8_000000, x0 - 2, y0 - 2, x0 - 2, y1 + 0);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x0 - 2, y1 + 1, x1 + 0, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, COL8_C6C6C6, x1 + 1, y0 - 2, x1 + 1, y1 + 1);
	boxfill8(sht->buf, sht->bxsize, c,           x0 - 1, y0 - 1, x1 + 0, y1 + 0);
	return;
}

/**
 * 关闭窗口
 * @param key_win		窗口
 * @param sht_win		任务a图层
 * @param cur_c			光标颜色
 * @param cur_x			光标位置
 * @return				光标颜色
 */
int keywin_off(struct SHEET *key_win, struct SHEET *sht_win, int cur_c, int cur_x) {
	// 使当前窗口变黑
	change_wtitle8(key_win, 0);
	// 如果当前窗口为任务a，删除光标
	if (key_win == sht_win) {
		// 删除光标
		cur_c = -1;
		boxfill8(sht_win->buf, sht_win->bxsize, COL8_FFFFFF, cur_x, 28, cur_x + 7, 43);
	}
	else {
		// 如果使命令窗口为活动窗口，则删除光标
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 3);
		}
	}
	return cur_c;
}

/**
 * 激活窗口
 * @param key_win		窗口
 * @param sht_win		当前窗口
 * @param cur_c		光标颜色
 * @return			光标颜色
 */
int keywin_on(struct SHEET *key_win, struct SHEET *sht_win, int cur_c) {
	// 使当前窗口变亮
	change_wtitle8(key_win, 1);
	// 如果当前窗口为任务a，显示光标
	if (key_win == sht_win) {
		cur_c = COL8_000000;
	}
	else {
		// 如果使命令窗口为活动窗口，则显示光标
		if ((key_win->flags & 0x20) != 0) {
			fifo32_put(&key_win->task->fifo, 2);
		}
	}
	return cur_c;
}

/**
 * 改变窗口标题颜色
 * @param sht		图层
 * @param act		窗口颜色，如果为1则是彩色，如果为0则是黑色
 */
void change_wtitle8(struct SHEET *sht, char act) {
	int x, y, xsize = sht->bxsize;
	char c, tc_new, tbc_nex, tc_old, tbc_old, *buf = sht->buf;
	// 窗口标题变黑
	if (act != 0) {
		tc_new = COL8_FFFFFF;
		tbc_nex = COL8_000084;
		tc_old = COL8_C6C6C6;
		tbc_old = COL8_848484;
	}
	// 窗口标题变亮
	else {
		tc_new = COL8_C6C6C6;
		tbc_nex = COL8_848484;
		tc_old = COL8_FFFFFF;
		tbc_old = COL8_000084;
	}
	// 改变窗口标题颜色
	for (y = 3; y <= 20; y++) {
		for (x = 3; x <= xsize - 4; x++) {
			c = buf[y * xsize + x];
			if (c == tc_old && x <= xsize - 22) {
				c = tc_new;
			}
			else if (c == tbc_old) {
				c = tbc_nex;
			}
			buf[y * xsize + x] = c;
		}
	}
	sheet_refresh(sht, 3, 3, xsize, 21);
	return;
}
