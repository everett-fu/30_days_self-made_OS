/*
 * Filename: winhelo2.c
 * Author: everett.fu
 * Created: 25-4-6
 * Last Modified: 25-4-6
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
int api_openwin(char *buf, int xsiz, int ysiz, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_end(void);
void api_closewin(int win);
int api_getkey(int mode);

char buf[150 * 50];

void HariMain(void)
{
	int win;
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win,  8, 36, 141, 43, 3 /*黄色*/);
	api_putstrwin(win, 28, 28, 0 /*黑色*/, 12, "hello, world");
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break;
		}
	}
	api_closewin(win);
	api_end();
}