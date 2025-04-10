/*
 * Filename: walk.c
 * Author: everett.fu
 * Created: 25-4-10
 * Last Modified: 25-4-10
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_initmalloc(void);
char *api_malloc(int size);
void api_refreshwin(int win , int x0, int y0, int x1, int y1);
void api_linewin(int win, int x0, int y0, int x1, int y1, int col);
void api_end(void);
void api_closewin(int win);
int api_getkey(int mode);
void api_boxfilwin(int win, int x0, int y0, int x1, int y1, int col);

void HariMain(void) {
	char *buf;
	int win, i, x, y;
	api_initmalloc();
	buf = api_malloc(160 * 100);
	win = api_openwin(buf, 160, 100, -1, "walk");
	api_boxfilwin(win, 6, 26, 154, 90, 0);
	x = 76;
	y = 56;

	api_putstrwin(win, x, y, 3, 1, "*");
	for (;;) {
		i = api_getkey(1);
		api_putstrwin(win, x, y, 0, 1, "*");
		if (i == '4' && x > 4) {
			x -= 8;
		}
		else if ( i == '6' && x < 148) {
			x += 8;
		}
		else if ( i == '2' && x < 148) {
			y += 8;
		}
		else if ( i == '8' && x < 148) {
			y -= 8;
		}
		else if (i == 0x0a) {
			break;
		}
		api_putstrwin(win, x, y, 3, 1, "*");
	}
	api_closewin(win);
	api_end();
}
