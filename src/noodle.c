/*
 * Filename: noodle.c
 * Author: everett.fu
 * Created: 25-4-17
 * Last Modified: 25-4-17
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */

#include <stdio.h>

void api_initmalloc(void);
char *api_malloc(int size);
int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);

void api_refreshwin(int win , int x0, int y0, int x1, int y1);
void api_end(void);
void api_closewin(int win);
int api_getkey(int mode);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
int api_alloctimer(void);
void api_inittimer(int timer, int data);

void api_settimer(int timer, int data);
void api_freetimer(int timer);

void HariMain(void) {
	char *buf;
	int win, timer;
	int hour = 0, minute = 0,second = 0;
	char s[30];
	api_initmalloc();
	buf = api_malloc(150 * 50);
	win = api_openwin(buf, 150, 50, -1, "noodle");
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for (;;) {
		// 显示时间
		sprintf(s, "%05d:%02d:%02d", hour, minute, second);
		api_boxfilwin(win, 28, 27, 115, 41,7);
		api_putstrwin(win, 28, 27, 0, 11,s);

		api_settimer(timer, 100);
		// 如果定时器没有超时，退出程序
		if (api_getkey(1) != 128) {
			break;
		}
		second++;
		if (second == 60){
			second = 0;
			minute++;
			if (minute == 60) {
				minute = 0;
				hour++;
			}
		}
	}
	api_end();
}
