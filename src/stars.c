/*
 * Filename: stars.c
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
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);
void api_initmalloc(void);
char *api_malloc(int size);
void api_point(int win, int x, int y, int col);
void api_end(void);
int api_getkey(int mode);
void api_closewin(int win);

int rand(void);

void HariMain(void) {
	char *buf;
	int win;
	api_initmalloc();
	buf = api_malloc(150 * 100);
	win = api_openwin(buf, 150, 100, -1, "stars");
	api_boxfilwin(win, 6, 26, 143, 93, 0);
	int i, x, y;
	for (i = 0; i < 50; i++) {
		x = (rand() % 137) + 6;
		y = (rand() % 67) + 26;
		api_point(win, x, y, 3);
	}
	for (;;) {
		if (api_getkey(1) == 0x0a) {
			break;
		}
	}
	api_closewin(win);
	api_end();
}
