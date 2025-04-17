/*
 * Filename: bootpack.c
 * Author: everett.fu
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个文件是操作系统的主文件，包含了操作系统的入口函数和一些初始化函数。
 *
 * Functions:
 * - HariMain: 主函数
 *
 * Usage:
 */

#include "bootpack.h"

void HariMain(void) {
	// 获取启动信息
	struct BOOTINFO *binfo = (struct BOOTINFO *) ADR_BOOTINFO;
	// 输入缓冲区，向键盘输出的缓冲区
	struct FIFO32 fifo, keycmd;
	// 临时变量，用来存储字符串
	char s[40];
	// 临时变量，用来存储数据与键盘信息
	int fifobuf[128], keycmd_buf[32];

	// 申请定时器
	struct TIMER *timer;
	// 鼠标坐标mx，my
	int mx, my;
	// 临时变量
	int i;
	// 鼠标数据
	struct MOUSE_DEC mdec;
	// 鼠标移动差值
	int mmx = -1, mmy = -1;
	// 鼠标当前点击的图层
	struct SHEET *sht = 0;
	// 内存总数
	unsigned int memtotal;
	// 内存管理结构体
	struct MEMMAN *memman = (struct MEMMAN *) MEMMAN_ADDR;

	// 图层控制器
	struct SHTCTL *shtctl;
	// 背景图层，鼠标图层，窗口图层，终端窗口图层
	struct SHEET *sht_back, *sht_mouse, *sht_win, *sht_cons;
	// 背景图层缓冲区，鼠标图层缓冲区，窗口图层缓冲区，终端窗口图层缓冲区
	unsigned char *buf_back, buf_mouse[256], *buf_win, *buf_cons;

	// 光标位置，光标颜色
	int cursor_x, cursor_c;

	// 创建任务a,b
	struct TASK *task_a, *task_cons;

	// 标志位，用来判断是否按下tab，用来切换任务窗口，存放任务的地址
	struct SHEET *key_win;
	// 标志位，用来判断shift是否按下
	int key_shift = 0;
	// 键盘锁定键状态
	// 第一个比特为ScrollLock，第二个比特为NumLock，第三个比特为CapsLock
	int key_leds = (binfo->leds >> 4) & 7;
	// 键盘控制器状态，当等于-1的时候代表可以发送，不等于-1的时候，键盘正在等待发送
	int keycmd_wait = -1;

	// 初始化GDT,IDT
	init_gdtidt();

	// 初始化PIC
	init_pic();

	// 初始化定时器
	init_pit();

	// 将中断标志设置为1，允许中断
	io_sti();

	// 初始化FIFO缓冲区
	fifo32_init(&fifo, 128, fifobuf, 0);
	// 向键盘输出的缓冲区
	fifo32_init(&keycmd, 32, keycmd_buf, 0);

	// 申请定时器，并初始化与设置定时器
	timer = timer_alloc();
	timer_init(timer, &fifo, 1);
	timer_settime(timer, 50);

	// 打开中断
	io_out8(PIC0_IMR, 0xf8);                // PIC1和键盘许可(11111000)
	io_out8(PIC1_IMR, 0xef);                // 鼠标许可(11101111)

	// 初始化键盘
	init_keyboard(&fifo, 256);
	// 激活鼠标
	enable_mouse(&fifo, 512, &mdec);
	// 初始化内存管理
	// 获取内存大小
	memtotal = memtest(0x00400000, 0xbfffffff);
	// 初始化内存管理
	memman_init(memman);
	memman_free(memman, 0x00001000,0x0009e000);
	memman_free(memman, 0x00400000,memtotal - 0x00400000);

	// 初始化任务
	task_a = task_init(memman);
	fifo.task = task_a;
	task_run(task_a, 1, 0);

	// 初始化调色板
	init_palette();

	// 初始化图层控制
	shtctl = shtctl_init(memman, binfo->vram, binfo->scrnx, binfo->scrny);
	*((int *)0x0fe4) = (int)shtctl;

	// 背景图层
	// 创建背景图层
	sht_back = sheet_alloc(shtctl);
	// 分配背景图层缓冲区
	buf_back = (unsigned char *) memman_alloc_4k(memman, binfo->scrnx * binfo->scrny);
	// 设置背景图层大小和透明色
	sheet_setbuf(sht_back, buf_back, binfo->scrnx, binfo->scrny, -1);
	// 将类Windows效果放置到背景图层之中
	init_screen8(buf_back, binfo->scrnx, binfo->scrny);

	// 鼠标图层
	// 创建鼠标图层
	sht_mouse = sheet_alloc(shtctl);
	// 设置鼠标图层大小和透明色
	sheet_setbuf(sht_mouse, buf_mouse, 16, 16, 99);
	// 将鼠标指针放置到鼠标图层之中
	init_mouse_cursor8(buf_mouse, 99);
	// 计算显示中间位置
	mx = (binfo->scrnx - 16) / 2;
	my = (binfo->scrny - 28 - 16) / 2;

	// 窗口a图层
	// 创建窗口图层
	sht_win = sheet_alloc(shtctl);
	// 分配窗口图层缓冲区
	buf_win = (unsigned char *) memman_alloc_4k(memman, 144 * 52);
	// 设置窗口图层大小和透明色
	sheet_setbuf(sht_win, buf_win, 144, 52, -1);
	// 将窗口放置到窗口图层之中
	make_window8(buf_win, 144, 52, "editor", 1);
	// 将文本框放置到窗口图层中
	make_textbox8(sht_win, 8, 28, 128, 16, COL8_FFFFFF);
	// 光标位置
	cursor_x = 8;
	// 光标颜色
	cursor_c = COL8_FFFFFF;
	// 初始化给窗口a
	key_win = sht_win;

	// 终端图层
	sht_cons = sheet_alloc(shtctl);
	buf_cons = (unsigned char *)memman_alloc_4k(memman, 256 * 165);
	sheet_setbuf(sht_cons, buf_cons, 256, 165, -1);
	make_window8(buf_cons, 256, 165, "console", 0);
	make_textbox8(sht_cons, 8, 28, 240, 128, COL8_000000);
	task_cons = task_alloc();
	// 为任务b的堆栈分配了64kb的内存，并计算出栈底的内存地址
	task_cons->tss.esp = memman_alloc_4k(memman, 64 * 1024) + 64 * 1024 - 12;
	// 设置任务b的堆栈信息
	task_cons->tss.eip = (int) &console_task;
	task_cons->tss.es = 1 * 8;
	task_cons->tss.cs = 2 * 8;
	task_cons->tss.ss = 1 * 8;
	task_cons->tss.ds = 1 * 8;
	task_cons->tss.fs = 1 * 8;
	task_cons->tss.gs = 1 * 8;
	*((int *)(task_cons->tss.esp + 4)) = (int)sht_cons;
	*((int *)(task_cons->tss.esp + 8)) = memtotal;
	sht_cons->task = task_cons;
	// 有光标
	sht_cons->flags |= 0x20;
	task_run(task_cons, 2, 2);
	struct CONSOLE *cons;

	// 背景色填充
	sheet_slide(sht_back, 0, 0);
	// 显示鼠标
	sheet_slide(sht_mouse, mx, my);
	// 显示窗口
	sheet_slide(sht_win, 8, 56);
	sheet_slide(sht_cons, 32, 4);
	// 设置背景图层高度
	sheet_updown(sht_back, 0);
	// 设置窗口图层高度
	sheet_updown(sht_win, 1);
	sheet_updown(sht_cons, 1);
	// 设置鼠标图层高度
	sheet_updown(sht_mouse, 10);

	// 为避免与键盘当前状态相冲突，先发送进行一次设置
	fifo32_put(&keycmd, KEYCMD_LED);
	fifo32_put(&keycmd, key_leds);

	// 系统主循环
	for (;;) {
		// 判断键盘输出缓冲是否有数据，有的话发送数据
		if (fifo32_status(&keycmd) > 0 && keycmd_wait < 0) {
			keycmd_wait = fifo32_get(&keycmd);
			// 等待键盘电路准备完备
			wait_KBC_sendready();
			io_out8(PORT_KEYDAT, keycmd_wait);
		}
		// 屏蔽中断
		io_cli();
		// 判断输入缓冲区是否有数据
		// 如果输入缓冲中没有任何的数据，则进入休眠状态
		if (fifo32_status(&fifo) == 0) {
			// 如果没有中断，自己休眠自己
			task_sleep(task_a);
			io_sti();
		}
		// 有中断
		else {
			i = fifo32_get(&fifo);
			io_sti();
			// 如果窗口被关闭，则聚焦到最上层一个窗口
			if (key_win->flags == 0) {
				key_win = shtctl->sheets[shtctl->top - 1];
				cursor_c = keywin_on(key_win, sht_win, cursor_c);
			}
			// 如果有键盘输入，则显示键盘输入
			if (i >=256 && i <= 511) {
				// 一般字符
				if (i < 256 + 0x80 && keytable0[i - 256] != 0) {
					// shift没有被按下
					if (key_shift == 0) {
						s[0] = keytable0[i - 256];
					}
					else {
						s[0] = keytable1[i - 256];
					}
					if (s[0] >= 'A' && s[0] <= 'Z') {
						// 如果大写锁定没有开启，并且没有按下shift，使用小写
						// 或者大写锁定开启了，并且按下了shift，使用小写
						if (((key_leds & 4) == 0 && key_shift == 0) || ((key_leds & 4) != 0 && key_shift != 0)) {
							s[0] += 0x20;
						}
					}
					// 给任务a的数据
					if (key_win == sht_win) {
						if (cursor_x < 128) {
							s[1] = 0;
							putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_FFFFFF, COL8_008484, s, 1);
							cursor_x += 8;
						}
					}
					// 给任务b的数据
					else {
						fifo32_put(&task_cons->fifo, s[0] + 256);
					}
				}
				// 其他字符
				else {
					// 退格键
					if (i == 256 + 0x0e) {
						// 任务a
						if (key_win == sht_win) {
							if (cursor_x > 8) {
								// 把光标的位置变成背景颜色，再改上一个字符的颜色
								putfonts8_asc_sht(sht_win, cursor_x, 28, COL8_000000, COL8_FFFFFF, " ", 1);
								cursor_x -=8;
							}
						}
						// 任务b
						else {
							fifo32_put(&task_cons->fifo, 8 + 256);
						}
					}
					// TAB键
					else if (i == 256 + 0x0f) {
						int j;
						cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
						j = key_win->height - 1;
						if (j == 0) {
							j = shtctl->top - 1;
						}
						key_win = shtctl->sheets[j];
						cursor_c = keywin_on(key_win, sht_win, cursor_c);
					}
					// 左Shift ON
					else if (i == 256 + 0x2a) {
						key_shift |= 1;
					}
					// 左Shift OFF
					else if (i == 256 + 0xaa) {
						key_shift &= ~1;
					}
					// 右Shift ON
					else if (i == 256 + 0x36) {
						key_shift |= 2;
					}
					// 右Shift OFF
					else if (i == 256 + 0xb6) {
						key_shift &= ~2;
					}
					// 大写锁定键
					else if (i == 256 + 0x3a) {
						// 切换锁定键
						key_leds ^= 4;
						fifo32_put(&keycmd, KEYCMD_LED);
						fifo32_put(&keycmd, key_leds);
					}
					// 数字锁定键
					else if (i == 256 + 0x45) {
						key_leds ^= 2;
						fifo32_put(&keycmd, KEYCMD_LED);
						fifo32_put(&keycmd, key_leds);
					}
					// ScrollLock锁定键
					else if (i == 256 + 0x46) {
						key_leds ^= 1;
						fifo32_put(&keycmd, KEYCMD_LED);
						fifo32_put(&keycmd, key_leds);
					}
					// 发送成功
					else if (i == 256 + 0xfa) {
						keycmd_wait = -1;
					}
					else if (i == 256 + 0xfe) {
						wait_KBC_sendready();
						io_out8(PORT_KEYDAT, keycmd_wait);
					}
					// 回车键
					else if (i == 256 + 0x1c) {
						// 发送到命令行窗口
						if (key_win != sht_win) {
							fifo32_put(&task_cons->fifo, 10 + 256);
						}
					}
					// shift+f1中止应用程序
					else if (i == 256 + 0x3b && key_shift != 0 && task_cons->tss.ss0 != 0) {
						cons = (struct CONSOLE *)*((int *)0xfec);
						cons_putstr(cons, "\nBreak(key):\n");
						io_cli();
						task_cons->tss.eax = (int)&task_cons->tss.esp0;
						task_cons->tss.eip = (int)asm_end_app;
						io_sti();
					}
					// 切换窗口
					else if (i == 256 + 0x57 && shtctl->top > 2) {
						sheet_updown(shtctl->sheets[1], shtctl->top - 1);
					}
				}
				// 重新显示光标
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
				}
				sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
			}
			// 如果有鼠标输入，则显示鼠标输入
			else if (i >= 512 && i <=767) {
				// 如果鼠标的数据接收完全
				if (mouse_decode(&mdec, i - 512) != 0) {
					// 鼠标指针的移动
					mx += mdec.x;
					my += mdec.y;
					if (mx < 0) {
						mx = 0;
					}
					if (my < 0) {
						my = 0;
					}
					if (mx > binfo->scrnx - 1) {
						mx = binfo->scrnx - 1;
					}
					if (my > binfo->scrny - 1) {
						my = binfo->scrny - 1;
					}
					// 显示鼠标
					sheet_slide(sht_mouse, mx, my);

					// 按下左键
					if ((mdec.btn & 0x01) != 0) {
						int x, y;
						int j;

						// 没有点击到窗口栏
						if (mmx < 0) {
							// 从上往下遍历窗口
							for (j = shtctl->top -1; j > 0; j--) {
								sht = shtctl->sheets[j];
								// 计算鼠标在该图层的偏移量
								x = mx - sht->vx0;
								y = my - sht->vy0;
								// 如果鼠标在窗口范围内
								if (x >= 0 && x < sht->bxsize && y >= 0 && y < sht->bysize) {
									// 不是透明区域
									if (sht->buf[y * sht->bxsize + x] != sht->col_inv) {
										// 设置图层高度为除鼠标的最高层
										sheet_updown(sht, shtctl->top - 1);
										// 鼠标所指的图层不是现在高亮窗口
										if (sht != key_win) {
											// 使现在高亮窗口变黑
											cursor_c = keywin_off(key_win, sht_win, cursor_c, cursor_x);
											// 使鼠标所指的窗口变亮
											key_win = sht;
											cursor_c = keywin_on(key_win, sht_win, cursor_c);
										}
										// 如果光标是在标题栏，并且不在关闭按钮上
										if (x >= 3 && x < sht->bxsize - 21 && y >= 3 && y < 21) {
											// 记录当前鼠标的位置
											mmx = mx;
											mmy = my;
										}
										// 如果点击到关闭按钮，关闭窗口，并结束应用程序
										else if (x >= sht->bxsize - 21 && x < sht->bxsize - 5 && y >= 5 && y < 19) {
											if ((sht->flags & 0x10) != 0) {
												cons = (struct CONSOLE *)*((int *)0xfec);
												cons_putstr(cons, "\nBreak(mouse):\n");
												io_cli();
												task_cons->tss.eax = (int)&(task_cons->tss.esp0);
												task_cons->tss.eip = (int)asm_end_app;
												io_sti();

											}
										}
										break;
									}
								}
							}
						}
						else {
							// 当进入这个条件时，鼠标一定按下，并且一定在窗口栏上
							// 这里的mx，my是鼠标是现在窗口上的坐标，与上面的if里的mx，my不是同一个
							// 这个的X，Y是两次鼠标坐标差值
							x = mx - mmx;
							y = my - mmy;
							sheet_slide(sht, sht->vx0 + x, sht->vy0 + y);
							mmx = mx;
							mmy = my;
						}
					}
					else {
						mmx = -1;
					}
//					if ((mdec.btn & 0x02) != 0) {
//						s[3] = 'R';
//					}
//					if ((mdec.btn & 0x04) != 0) {
//						s[2] = 'C';
//					}
				}
			}
			// 光标寄存器
			else if (i <= 1) {
				// 光标闪烁
				if (i == 1)
				{
					timer_init(timer, &fifo, 0);
					if (cursor_c >= 0)
					{
						cursor_c = COL8_000000;
					}
				}
				else {
					timer_init(timer, &fifo, 1);
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				timer_settime(timer, 50);
				if (cursor_c >= 0) {
					boxfill8(sht_win->buf, sht_win->bxsize, cursor_c, cursor_x, 28, cursor_x + 7, 43);
					sheet_refresh(sht_win, cursor_x, 28, cursor_x + 8, 44);
				}
			}
		}
	}
}
