void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);

#define COL8_000000 0            // 0: 黑
#define COL8_FF0000 1            // 1: 亮红
#define COL8_00FF00 2            // 2: 亮緑
#define COL8_FFFF00 3            // 3: 亮黄
#define COL8_0000FF 4            // 4: 亮蓝
#define COL8_FF00FF 5            // 5: 亮紫
#define COL8_00FFFF 6            // 6: 浅亮蓝
#define COL8_FFFFFF 7            // 7: 白
#define COL8_C6C6C6 8            // 8: 亮灰
#define COL8_840000 9            // 9: 暗红
#define COL8_008400 10            // 10: 暗緑
#define COL8_848400 11            // 11: 暗黄
#define COL8_000084 12            // 12: 暗青
#define COL8_840084 13            // 13: 暗紫
#define COL8_008484 14            // 14: 浅暗蓝
#define COL8_848484 15            // 15: 暗灰

void HariMain(void) {
	unsigned char *vram;
	int xsize, ysize;
	short *binfo_scrnx, *binfo_scrny;
	int *binfo_vram;

	init_palette();
	binfo_scrnx = (short *)0x0ff4;
	binfo_scrny = (short *)0x0ff6;
	binfo_vram = (int *)0x0ff8;
	xsize = *binfo_scrnx;
	ysize = *binfo_scrny;
	vram = (unsigned char *) *binfo_vram;

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

	for (;;)
		io_hlt();
}

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
 * @param xsize		x轴大小
 * @param c			颜色
 * @param x0		x0
 * @param y0		y0
 * @param x1		x1
 * @param y1		y1
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