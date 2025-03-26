/*
 * Filename: console.c
 * Author: everett.fu
 * Created: 25-3-25
 * Last Modified: 25-3-25
 * Description:
 * 这个文件是命令行窗口
 *
 *Functions:
 * - console_task: 任务b
 * - cons_newline: 换行函数
 *
 * Usage:
 */

#include <stdio.h>
#include <string.h>
#include "bootpack.h"

/**
 * 任务b
 * @param sht_back		要显示的图层地址
 * @param memtotal		内存地址
 */
void console_task(struct SHEET *sheet, unsigned int memtotal){
	// 缓冲区数据
	int fifobuf[128];
	// 界面刷新定时器
	struct TIMER *timer;
	// 获取当前任务的地址
	struct TASK *task = task_now();
	// 临时变量，字符位置x，字符位置y，字符颜色
	int i, cursor_x = 16, cursor_y = 28, cursor_c = -1;
	// 临时变量
	int x, y;
	// 临时变量，用于存储字符
	char s[30], cmdline[30], *p;
	struct MEMMAN *memman = (struct MEMMAN *)MEMMAN_ADDR;

	// 申请地址空间，用于保存fat表
	int *fat = (int *)memman_alloc_4k(memman, 4 * 2880);

	fifo32_init(&task->fifo, 128, fifobuf, task);
	timer = timer_alloc();
	timer_init(timer, &task->fifo, 1);
	timer_settime(timer, 50);

	putfonts8_asc_sht(sheet, 8, 28, COL8_FFFFFF, COL8_000000, ">", 1);

	// 命令行缓冲区
	struct FILEINFO *finfo = (struct FILEINFO *)(ADR_DISKIMG + 0x002600);
	// 将磁盘中的fat表解密
	file_readfat(fat, (unsigned char *)(ADR_DISKIMG + 0x000200));

	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *)ADR_GDT;

	for (;;) {
		io_cli();
		if (fifo32_status(&task->fifo) == 0) {
			task_sleep(task);
			io_sti();
		}
		else {
			i = fifo32_get(&task->fifo);
			io_sti();
			// 光标定时器
			if (i <= 1) {
				if (i == 1) {
					timer_init(timer, &task->fifo, 0);
					if (cursor_c >= 0) {
						cursor_c = COL8_FFFFFF;
					}
				}
				else {
					timer_init(timer, &task->fifo, 1);
					if (cursor_c >= 0) {
						cursor_c = COL8_000000;
					}
				}
				timer_settime(timer, 50);
			}
			// 打开光标
			else if (i == 2) {
				cursor_c = COL8_FFFFFF;
			}
			// 关闭光标
			else if (i == 3) {
				boxfill8(sheet->buf, sheet->bxsize, COL8_000000, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
				cursor_c = -1;
			}
			// 如果有键盘输入，则显示键盘输入
			if (i >=256 && i <= 511) {
				// 退格符
				if (i == 8 + 256) {
					if (cursor_x > 16) {
						// 把光标的位置变成背景颜色，再改上一个字符的颜色
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
						cursor_x -=8;
					}
				}
				// 回车键
				else if (i == 10 + 256) {
					putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);

					cmdline[cursor_x / 8 -2] = 0;
					cursor_y = cons_newline(cursor_y, sheet);
					// free命令，显示剩余内存
					if (strcmp(cmdline, "free") == 0) {
						sprintf(s, "     total    used    free");
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);

						sprintf(s, "Mem: %dMB     %dKB  %dKB", memtotal / (1024 * 1024), memtotal / 1024 - memman_total(memman) / 1024 , memman_total(memman) / 1024);
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}
					// clear命令，清屏
					else if (strcmp(cmdline, "clear") == 0) {
						for (y = 28; y < 28 + 128; y++) {
							for (x = 8; x < 8 + 240; x++) {
								sheet->buf[x + y * sheet->bxsize] = COL8_000000;
							}
						}
						sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
						cursor_y = 28;
					}
					// 查看文件
					else if (strcmp(cmdline, "ls") == 0) {
						// 最多存放224个文件
						for (x = 0; x < 224; x++) {
							// 这段不含文件信息
							if (finfo[x].name[0] == 0x00) {
								break;
							}
							// 文件没有被删除
							else if (finfo[x].name[0] != 0xe5) {
								// 不是归档文件或者目录
								if ((finfo[x].type & 0x18) == 0)  {
									sprintf(s, "filename.ext %7d", finfo[x].size);
									for (y = 0; y < 8; y++) {
										s[y] = finfo[x].name[y];
									}
									s[9] = finfo[x].ext[0];
									s[10] = finfo[x].ext[1];
									s[11] = finfo[x].ext[2];
									putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, s, 30);
									cursor_y = cons_newline(cursor_y, sheet);
								}
							}
						}
						cursor_y = cons_newline(cursor_y, sheet);
					}
					// cat命令，只比较前四个字符，后面的为文件名
					else if (strncmp(cmdline, "cat ", 4) == 0) {
						// 将字符串S用空格填充
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}

						// 将文件名读取到字符串S中，文件名8位，不满的用空格填充，第8位开始为后缀名，后缀名3位
						for (y = 0, x = 4; y < 11 && cmdline[x] != 0; x++) {
							if (cmdline[x] == '.' && y <= 8) {
								y = 8;
							}
							else {
								s[y] = cmdline[x];
								// 如果是小写字母，将字母转换成大写
								if (s[y] >= 'a' && s[y] <= 'z') {
									s[y] -= 0x20;
								}
								y++;
							}
						}
						// 查找文件
						// 与文件格式中一个个对比
						for (x = 0; x < 224; x++) {
							// 是否找到文件，为0则是没有找到，为1则找到了文件
							char file_flag = 1;
							// 该数据段不含内容
							if (finfo[x].name[0] == 0x00) {
								file_flag = 0;
								break;
							}
							// 为不是目录或者归档文件
							if ((finfo[x].type & 0x18) == 0) {
								// 文件名不能匹配上
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										file_flag = 0;
										break;
									}
								}
								// 可以匹配上，跳出循环，开始显示字符
								if (file_flag == 1) {
									break;
								}
							}
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							// 加载文件
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *)(ADR_DISKIMG + 0x003e00));
							cursor_x = 8;
							for (y = 0; y < finfo[x].size; y++) {
								// 一次输出一个字符
								s[0] = p[y];
								s[1] = 0;
								// 制表符
								if (s[0] == 0x09) {
									for (;;) {
										putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, " ", 1);
										cursor_x += 8;
										// 如果到界面最后，进行换行
										if (cursor_x == 8 + 240) {
											cursor_x = 8;
											cursor_y = cons_newline(cursor_y, sheet);
										}
										// 如果光标位置是32的倍数，则跳出循环
										// 减8是因为光标位置是从8开始的，开始有8个像素是窗口的宽度
										if (((cursor_x - 8) & 0x1f) == 0) {
											break;
										}
									}
								}
								// 换行符
								else if (s[0] == 0x0a) {
									cursor_x = 8;
									cursor_y = cons_newline(cursor_y, sheet);
								}
								// 回车符
								else if (s[0] == 0x0d) {
								}
								// 一般字符
								else {
									putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
									cursor_x += 8;
									// 如果到界面最后，进行换行
									if (cursor_x == 8 + 240) {
										cursor_x = 8;
										cursor_y = cons_newline(cursor_y, sheet);
									}
								}
							}
							// 释放内存
							memman_free_4k(memman, (int)p, finfo[x].size);
						}
						// 没有找到文件
						else {
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);
					}
					// hlt命令，使窗口休眠
					else if (strcmp(cmdline, "hlt") == 0) {
						for (y = 0; y < 11; y++) {
							s[y] = ' ';
						}
						s[0] = 'H';
						s[1] = 'L';
						s[2] = 'T';
						s[8] = 'H';
						s[9] = 'R';
						s[10] = 'B';
						for (x = 0; x < 224; x++) {
							// 是否找到文件，为0则是没有找到，为1则找到了文件
							char file_flag = 1;
							if (finfo[x].name[0] == 0x00) {
								file_flag = 0;
								break;
							}
							// 为不是目录或者归档文件
							if ((finfo[x].type & 0x18) == 0) {
								// 文件名不能匹配上
								for (y = 0; y < 11; y++) {
									if (finfo[x].name[y] != s[y]) {
										file_flag = 0;
										break;
									}
								}
								if (file_flag == 1) {
									// 找到文件，跳出循环
									break;
								}
							}
						}
						if (x < 224 && finfo[x].name[0] != 0x00) {
							p = (char *) memman_alloc_4k(memman, finfo[x].size);
							file_loadfile(finfo[x].clustno, finfo[x].size, p, fat, (char *) (ADR_DISKIMG + 0x003e00));
							set_segmdesc(gdt + 1003, finfo[x].size - 1, (int) p, AR_CODE32_ER);
							// 跳转到指定位置
							farjmp(0, 1003 * 8);
							memman_free_4k(memman, (int) p, finfo[x].size);
						} else {
							putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "File not found.", 15);
							cursor_y = cons_newline(cursor_y, sheet);
						}
						cursor_y = cons_newline(cursor_y, sheet);

					}
					// 不是命令，也不是空行，即为错误命令
					else if (cmdline[0] != 0) {
						putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, "command not found!", 19);
						cursor_y = cons_newline(cursor_y, sheet);
						cursor_y = cons_newline(cursor_y, sheet);
					}

					putfonts8_asc_sht(sheet, 8, cursor_y, COL8_FFFFFF, COL8_000000, ">", 1);
					cursor_x = 16;
				}
				// 一般字符
				else {
					if ( cursor_x < 240) {
						s[0] = i - 256;
						s[1] = 0;
						cmdline[cursor_x / 8 -2] = i - 256;
						putfonts8_asc_sht(sheet, cursor_x, cursor_y, COL8_FFFFFF, COL8_000000, s, 1);
						cursor_x += 8;
					}
				}
			}
			if (cursor_c >= 0) {
				boxfill8(sheet->buf, sheet->bxsize, cursor_c, cursor_x, cursor_y, cursor_x + 7, cursor_y + 15);
			}
			sheet_refresh(sheet, cursor_x, cursor_y, cursor_x + 8, cursor_y + 16);
		}
	}
}

/**
 * 换行函数
 * @param cursor_y		光标位置
 * @param sheet			图层
 * @return				返回当前光标纵坐标
 */
int cons_newline(int cursor_y, struct SHEET *sheet) {
	int x, y;
	// 如果没有超出窗口纵向范围
	if (cursor_y < 28 + 112) {
		cursor_y += 16;
	}
	// 超出窗口纵向范围
	else {
		// 将窗口内容向上移动
		for (y = 28; y < 28 + 112; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = sheet->buf[x + (y + 16) * sheet->bxsize];
			}
		}
		// 清除最后一行
		for (y = 28 + 112; y < 28 + 128; y++) {
			for (x = 8; x < 8 + 240; x++) {
				sheet->buf[x + y * sheet->bxsize] = COL8_000000;
			}
		}
		// 显示命令提示符>
		sheet_refresh(sheet, 8, 28, 8 + 240, 28 + 128);
	}
	// 返回当前光标纵坐标
	return cursor_y;
}
