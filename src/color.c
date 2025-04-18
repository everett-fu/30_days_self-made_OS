/*
 * Filename: color.c
 * Author: everett.fu
 * Created: 25-4-18
 * Last Modified: 25-4-18
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
void api_initmalloc(void);
char *api_malloc(int size);
int api_openwin(char *buf, int xsize, int ysize, int col_inv, char *title);
void api_end(void);
void api_getkey(int mode);
void api_refreshwin(int win, int x0, int y0, int x1, int y1);

void HariMain(void) {
	char *buf;
	int win, x, y, r, g, b;
	api_initmalloc();
	buf = api_malloc(144 * 164);
	win = api_openwin(buf, 144, 164, -1, "color");
	for (y = 0; y < 128; y++) {
		for (x = 0; x < 128; x++) {
			r = x * 2;
			g = y * 2;
			b = 0;
			// /43是将255色阶映射到6色阶
			buf[(x + 8) + (y + 28) * 144] = 16 + (r / 43) + (g / 43) * 6 + (b / 43) * 36;
		}
	}
	api_refreshwin(win, 8, 28, 136, 156);
	// 按下人任意键结束任务
	api_getkey(1);
	api_end();
}