void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen(unsigned char *vram, int xsize, int ysize);
void putfont8(unsigned char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, unsigned char *s);

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

// 创建一个结构体，用来保存启动信息
struct BOOTINFO {
	// cyls: 启动区信息, leds: 键盘状态, vmode: 颜色数目, reserve: 颜色位数
	char cyls, leds, vmode, reserve;
	// scrnx: x分辨率, scrny: y分辨率
	short scrnx, scrny;
	// 显存地址
	unsigned char *vram;
};

void HariMain(void) {
	struct BOOTINFO *binfo = (struct BOOTINFO *) 0x0ff0;

	// 初始化调色板
	init_palette();

	// 显示类Windows效果
	init_screen(binfo->vram, binfo->scrnx, binfo->scrny);

	// 显示字符串
	putfonts8_asc(binfo->vram, binfo->scrnx,  8,  8, COL8_FFFFFF, "ABC 123");
	putfonts8_asc(binfo->vram, binfo->scrnx, 31, 31, COL8_000000, "Haribote OS.");
	putfonts8_asc(binfo->vram, binfo->scrnx, 30, 30, COL8_FFFFFF, "Haribote OS.");

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

/**
 * 绘制类Windows效果
 * @param vram		显存地址
 * @param xsize		宽
 * @param ysize		高
 */
void init_screen(unsigned char *vram, int xsize, int ysize) {
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
}

void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, unsigned char *s) {
	// 导入字符集
	extern char hankaku[4096];
	// 循环字符串，一直到字符串结束
	for (; *s != 0x00; s++) {
		putfont8(vram, xsize, x, y, c, hankaku + *s * 16);
		x += 8;
	}
}
