/*
 * Filename: winhelo3.c
 * Author: everett.fu
 * Created: 25-4-9
 * Last Modified: 25-4-9
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */

int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_putstrwin(int win, int x, int y, int col, int len, char *str);

void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_end(void);
void api_closewin(int win);
int api_getkey(int mode);

void HariMain(void) {
	char *buf;
	int win;
	api_initmalloc();
	buf = api_malloc(150 * 50);
	win = api_openwin(buf, 150, 50, -1, "hello");
	api_boxfilwin(win, 8, 36, 141, 43, 6);
	api_putstrwin(win, 28, 28, 0, 12, "hello, world");
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break;
		}
	}
	api_closewin(win);
	api_end();
}