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
void io_sti(void);
void io_stihlt(void);
int io_in8(int port);
void io_out8(int port, int data);
int io_load_eflags(void);
void io_store_eflags(int eflags);
void load_gdtr(int limit, int addr);
void load_idtr(int limit, int addr);
void asm_inthandler20(void);
void asm_inthandler21(void);
void asm_inthandler27(void);
void asm_inthandler2c(void);
int load_cr0(void);
void store_cr0(int cr0);
unsigned int memtest_sub(unsigned int start, unsigned int end);
void load_tr(int tr);
void farjmp(int eip, int cs);

// fifo.c
struct FIFO32 {
	// 缓冲区地址
	int *buf;
	// 下一个写入位置，下一个读取位置，缓冲区大小，缓冲区剩余大小，缓冲区溢出标志
	int next_w, next_r, size, free, flags;
	// 当有输入的时候唤醒以下程序
	struct TASK *task;
};
void fifo32_init(struct FIFO32 *fifo, int size, int *buf, struct TASK *task);
int fifo32_put(struct FIFO32 *fifo, int data);
int fifo32_get(struct FIFO32 *fifo);
int fifo32_status(struct FIFO32 *fifo);

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
#define AR_INTGATE32	0x008e
#define AR_TSS32		0x0089

// int.c
void init_pic(void);
void inthandler27(int *esp);
#define PIC0_ICW1		0x0020
#define PIC0_OCW2		0x0020
#define PIC0_IMR		0x0021
#define PIC0_ICW2		0x0021
#define PIC0_ICW3		0x0021
#define PIC0_ICW4		0x0021
#define PIC1_ICW1		0x00a0
#define PIC1_OCW2		0x00a0
#define PIC1_IMR		0x00a1
#define PIC1_ICW2		0x00a1
#define PIC1_ICW3		0x00a1
#define PIC1_ICW4		0x00a1

// keyboard.c
void inthandler21(int *esp);
void wait_KBC_sendready(void);
void init_keyboard(struct FIFO32 *fifo, int data0);
#define PORT_KEYDAT 0x0060
#define PORT_KEYCMD 0x0064
// 键盘字符
static char keytable[0x59] = {
	0, 0, '1', '2', '3', '4', '5', '6', '7', '8', '9', '0', '-', '=', 0, 0,
	'Q', 'W', 'E', 'R', 'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', 0, 0, 'A', 'S',
	'D', 'F', 'G', 'H', 'J', 'K', 'L', ';', '\'', '`', 0, '|', 'Z', 'X', 'C', 'V',
	'B', 'N', 'M', ',', '.', '/', 0, '*', 0, ' ', 0, 0, 0, 0, 0, 0,
	0, 0, 0, 0, 0, 0, 0, '7', '8', '9', '-', '4', '5', '6', '+', '1',
	'2', '3', '0', '.', 0, 0, 0, 0, 0
};

// mouse.c
struct MOUSE_DEC {
	// 鼠标符，鼠标状态
	unsigned char buf[3], phase;
	// x: 鼠标移动x坐标, y: 鼠标移动y坐标, btn: 按钮状态
	int x, y, btn;
};
void inthandler2c(int *esp);
void enable_mouse(struct FIFO32 *fifo, int data0, struct MOUSE_DEC *mdec);
int mouse_decode(struct MOUSE_DEC *mdec, unsigned char data);

// memory.c
#define MEMMAN_FREES 4090
#define MEMMAN_ADDR 0x003c0000
// 可用内存信息
struct FREEINFO {
	// 内存地址，内存大小
	unsigned int addr, size;
};
// 内存管理
struct MEMMAN {
	// 可用信息数量，最大frees数量，释放失败内存大小总和，释放失败次数
	int frees, maxfrees, lostsize, losts;
	struct FREEINFO free[MEMMAN_FREES];
};
unsigned int memtest(unsigned int start, unsigned int end);
void memman_init(struct MEMMAN *man);
unsigned int memman_total(struct MEMMAN *man);
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size);
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size);
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size);
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size);

// sheet.c
#define MAX_SHEETS 256
struct SHEET {
	// buf: 图层的显存地址
	unsigned char *buf;
	// bxsize: 图层的x分辨率，bysize: 图层的y分辨率，vx0: 图层的x坐标，vy0: 图层的y坐标，col_inv: 图层的透明色，height: 图层的高度，flags: 图层是否被使用
	int bxsize, bysize, vx0, vy0, col_inv, height, flags;
	struct SHTCTL *ctl;
};
struct SHTCTL {
	// vram: 显存地址
	unsigned char *vram, *map;
	// xsize: x分辨率，ysize: y分辨率，top: 最上层图层的高度
	int xsize, ysize, top;
	// sheets: 每个图层内容信息的地址
	struct SHEET *sheets[MAX_SHEETS];
	// sheets0: 每个图层结构体的信息
	struct SHEET sheets0[MAX_SHEETS];
};
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize);
struct SHEET *sheet_alloc(struct SHTCTL *ctl);
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv);
void sheet_updown(struct SHEET *sht, int height);
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1);
void sheet_slide(struct SHEET *sht, int vx0, int vy0);
void sheet_free(struct SHEET *sht);
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1);
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0);

// timer.c
#define MAX_TIMER 500
// 计时器
struct TIMER {
	// 指向下一个定时器的指针
	struct TIMER *next_timer;
	// 每个定时器超时时间,该定时器状态
	unsigned int timeout, flags;
	// 每个定时器超时以后需要发送数据到的缓冲区
	struct FIFO32 *fifo;
	// 每个定时器超时以后需要发送的数据
	int data;
};

// 计时器控制器
struct TIMERCTL {
	// 计时器当前时间，下一个超时时间，有几个定时器正在使用
	unsigned int count, next_timeout;
	// 排序好的定时器，按照超时时间从小到大排序
	struct TIMER *timer_head;
	// 原始定时器
	struct TIMER timers0[MAX_TIMER];
};
extern struct TIMERCTL timerctl;
void init_pit(void);
void inthandler20(int *esp);
struct TIMER * timer_alloc(void);
void timer_free(struct TIMER *timer);
void timer_init(struct TIMER *timer, struct FIFO32 *fifo, int data);
void timer_settime(struct TIMER *timer, unsigned int timeout);

// mtask.c
// 最大任务数量
#define MAX_TASKS 1000
// 每个任务列表最大任务数量
#define MAX_TASKS_LV 100
// 最多拥有任务列表的数量
#define MAX_TASKLEVELS 10
// 定义GDT从几号开始分配TSS
#define TASK_GDT0 3
// 关闭任务
#define TASK_FLAGS_CLOSE 0
// 启用任务
#define TASK_FLAGS_ALLOC 1
// 任务使用中
#define TASK_FLAGS_USING 2
// 任务状态相关的段
// 用于保存所有的寄存器信息与任务设置相关信息
// 用于多任务的切换
struct TSS32 {
	// 任务设置相关信息，除了backlink会被写入，其他的几个寄存器不会被写入
	int backlink, esp0, ss0, esp1, ss1, esp2, ss2, cr3;
	// 32位寄存器
	int eip, eflags, eax, ecx, edx, ebx, esp, ebp, esi, edi;
	// 16位寄存器
	int es, cs, ss, ds, fs, gs;
	// 任务设置相关信息
	int ldtr, iomap;
};

// 单个任务数据
struct TASK {
	// 任务的GDT的编号, 任务状态
	int sel, flags;
	// 当前任务所在的任务队列，任务运行时间，单位ms
	int level, priority;
	// 任务缓冲区
	struct FIFO32 fifo;
	// 任务状态相关的段
	struct TSS32 tss;
};

// 任务队列
struct TASKLEVEL {
	// 当前队列正在运行的数量
	int running_num;
	// 当前任务运行任务的编号
	int now_task;
	// 任务所在的地址
	struct TASK *tasks[MAX_TASKS_LV];
};

// 任务控制器
struct TASKCTL {
	// 当前活动的任务队列
	int now_lv;
	// 下次切换任务是是否需要改变任务队列
	char lv_change;
	// 任务队列
	struct TASKLEVEL level[MAX_TASKLEVELS];
	// 所有的任务存在的位置
	struct TASK tasks0[MAX_TASKS];
};

extern struct TASKCTL *taskctl;
extern struct TIMER *task_timer;
struct TASK * task_init(struct MEMMAN *memman);
struct TASK * task_alloc(void);
void task_run(struct TASK *task, int level, int priority);
void task_switch(void);
void task_sleep(struct TASK *task);
struct TASK *task_now(void);
void task_add(struct TASK *task);
void task_remove(struct TASK *task);
void task_switchsub(void);
void task_idle(void);
