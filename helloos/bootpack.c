void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);

void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);

void HariMain(void)
{
	int i;
	char *p;

	init_palette();	// 设定调色板

	for (i = 0xa0000; i <= 0xaffff; i++) {
		p = (char *)i;
		*p = i & 0x0f;
//		*p = 15;
	}
	for (;;)
		io_hlt();
}

void init_palette(void) {
	static unsigned char table_rgb[16 * 3] = {
		0x00, 0x00, 0x00,		// 黑
		0xff, 0x00, 0x00,		// 亮红
		0x00, 0xff, 0x00,		// 亮緑
		0xff, 0xff, 0x00,		// 亮黄
		0x00, 0x00, 0xff,	// 亮蓝
		0xff, 0x00, 0xff,	// 亮紫
		0x00, 0xff, 0xff,	// 浅亮蓝
		0xff, 0xff, 0xff,	// 白
		0xc6, 0xc6, 0xc6,	// 亮灰
		0x84, 0x00, 0x00,	// 暗红
		0x00, 0x84, 0x00,	// 暗緑
		0x84, 0x84, 0x00,	// 暗黄
		0x00, 0x00, 0x84,	// 暗青
		0x84, 0x00, 0x84,	// 暗紫
		0x00, 0x84, 0x84,	// 浅暗蓝
		0x84, 0x84, 0x84		// 暗灰
	};
	set_palette(0, 15, table_rgb);
	return;
}

/**
 * 设置调色板
 * @param start
 * @param end
 * @param rgb
 */
void set_palette(int start, int end, unsigned char *rgb) {
	int i, eflags;
	// 中断标志
	eflags = io_load_eflags();

	// 屏蔽中断
	io_cli();
	io_out8(0x03c8, start);
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