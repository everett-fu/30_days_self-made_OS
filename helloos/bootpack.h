/*
 * Filename: bootpack.h
 * Author: everett.fu
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个文件是操作系统的头文件，包含了操作系统的入口函数和一些初始化函数。
 *
 * Functions:
 *
 * Usage:
 */

// ashmhead.nas
// 创建一个结构体，用来保存启动信息
struct BOOTINFO {
	// cyls: 启动区信息, leds: 键盘状态, vmode: 颜色数目, reserve: 颜色位数
	char cyls, leds, vmode, reserve;
	// scrnx: x分辨率, scrny: y分辨率
	short scrnx, scrny;
	// 显存地址
	unsigned char *vram;
};
#define ADR_BOOTINFO    0x00000ff0

// naskfunc.nas
void io_hlt(void);
void io_cli(void);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);

// graphic.c
void init_palette(void);
void set_palette(int start, int end, unsigned char *rgb);
void boxfill8(unsigned char *vram, int xsize, unsigned char c, int x0, int y0, int x1, int y1);
void init_screen8(unsigned char *vram, int xsize, int ysize);
void putfont8(unsigned char *vram, int xsize, int x, int y, char c, char *font);
void putfonts8_asc(unsigned char *vram, int xsize, int x, int y, char c, unsigned char *s);
void init_mouse_cursor8(char *mouse, char bc);
void putblock8_8(char *vram, int vxsize, int pxsize, int pysize, int px0, int py0, char *buf, int bxsize);
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


// dsctbl.c
// GDT结构体
struct SEGMENT_DESCRIPTOR {
	// 三段base都是存放段的基址，limit存放页的大小与权限，access_right存放访问权限
	// limit_high8位中高四位存放权限，低四位存放页的大小
	// 段基址一共32位，即limit_low(16位)+limit_mid(8位)+limit_high(8位)
	// 段上限一共20位，用页来表示，一个页面4k，大小为1M(20位)*4k=4GB。
	// 20位为limit_low(16位)+limit_high低四位(4位)
	// 权限一共12位，即limit_high高四位(4位)+access_right(8位)
	short limit_low, base_low;
	char base_mid, access_right;
	char limit_high, base_high;
};

// IDT结构体
struct GATE_DESCRIPTOR {
	short offset_low, selector;
	char dw_count, access_right;
	short offset_high;
};
void init_gdtidt(void);
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar);
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar);
#define ADR_IDT			0x0026f800
#define LIMIT_IDT		0x000007ff
#define ADR_GDT			0x00270000
#define LIMIT_GDT		0x0000ffff
#define ADR_BOTPAK		0x00280000
#define LIMIT_BOTPAK	0x0007ffff
#define AR_DATA32_RW	0x4092
#define AR_CODE32_ER	0x409a
