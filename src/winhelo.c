/*
 * Filename: winhelo.c
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

int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);
char buf[150 * 50];

void HariMain(void) {
	int win;
	// 返回句柄，用于控制窗口
	win = api_openwin(buf, 150, 50, -1, "hello");
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break;
		}
	}
	api_closewin(win);
	api_end();
}