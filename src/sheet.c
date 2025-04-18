/*
 * Filename: sheet.c
 * Author: everett.fu
 * Created: 24-9-5
 * Last Modified: 24-9-5
 * Description:
 * 这个文件是图层的实现文件，包含了图层的初始化和显示等功能。
 *
 * Functions:
 * - shtctl_init: 初始化图层控制
 * - sheet_alloc: 找到一个未使用的图层
 * - sheet_setbuf: 设置图层的缓冲区大小和透明色
 * - sheet_updown: 设置图层的高度
 * - sheet_refresh: 刷新图层
 * - sheet_slide: 移动图层
 * - sheet_free: 释放图层
 * - sheet_refreshsub: 刷新图层
 * - sheet_refreshmap: 刷新图层
 *
 * Usage:
 */
#include "bootpack.h"
// 正在使用的图层标记成1
#define SHEET_USE 1

/**
 * 初始化图层控制
 * @param memman 内存地址
 * @param vram 显存地址
 * @param xsize 屏幕的x轴大小
 * @param ysize 屏幕的y轴大小
 * @return 返回分配好的图层地址
 */
struct SHTCTL *shtctl_init(struct MEMMAN *memman, unsigned char *vram, int xsize, int ysize) {
	struct SHTCTL *ctl;
	int i;
	ctl = (struct SHTCTL *) memman_alloc_4k(memman, sizeof(struct SHTCTL));
	ctl->map = (unsigned char *) memman_alloc_4k(memman, xsize * ysize);

	if (ctl->map != 0 && ctl != 0) {
		ctl->vram = vram;
		ctl->xsize = xsize;
		ctl->ysize = ysize;
		// 没有图层
		ctl->top = -1;
		// 将所有图层标记成未使用
		for (i = 0; i < MAX_SHEETS; i++) {
			ctl->sheets0[i].flags = 0;
			ctl->sheets0[i].ctl = ctl;
		}
	}
	else{
		memman_free_4k(memman, (int)ctl, sizeof(struct SHTCTL));
	}

	return ctl;
}

/**
 * 找到一个未使用的图层，返回图层地址，找到这个这个图层，高度为-1，不显示
 * @param ctl 图层控制结构体
 * @return 找到一个未使用的图层
 */
struct SHEET *sheet_alloc(struct SHTCTL *ctl) {
	struct SHEET *sht;
	int i;
	// 遍历所有的图层，找到未使用的图层
	for (i = 0; i < MAX_SHEETS; i++) {
		if (ctl->sheets0[i].flags == 0) {
			sht = &ctl->sheets0[i];
			sht->flags = SHEET_USE;
			sht->height = -1; // 不显示
			sht->task = 0;
			return sht;
		}
	}
	return 0; // 所有图层都在使用中
}

/**
 * 设置图层的缓冲区大小和透明色
 * @param sht 图层
 * @param buf 缓冲区
 * @param xsize x轴大小
 * @param ysize y轴大小
 * @param col_inv 透明色
 */
void sheet_setbuf(struct SHEET *sht, unsigned char *buf, int xsize, int ysize, int col_inv) {
	sht->buf = buf;
	sht->bxsize = xsize;
	sht->bysize = ysize;
	sht->col_inv = col_inv;
	return;
}

/**
 * 设置图层的高度
 * @param sht 需要设置的图层
 * @param height 高度
 * @return
 */
void sheet_updown(struct SHEET *sht, int height) {
	struct SHTCTL *ctl = sht->ctl;
	int h, old = sht->height;
	// 判断height是否超出范围
	if (height > ctl->top + 1) {
		height = ctl->top + 1;
	}
	else if (height < -1) {
		height = -1;
	}

	sht->height = height;

	// 如果重新设定图层的高度小于之前的高度，将height~old之前的图层提升一层
	if (height < old) {
		// 如果有高度，则显示
		if (height >= 0) {
			for (h = old; h > height; h--) {
				ctl->sheets[h] = ctl->sheets[h - 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height + 1, old);
		}
		// 如果没有高度，则隐藏起来
		else {
			if (old < ctl->top) {
				for (h = old; h < ctl->top; h++) {
					ctl->sheets[h] = ctl->sheets[h + 1];
					ctl->sheets[h]->height = h;
				}
			}
			ctl->top--;
			sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0);
			sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, 0, old - 1);
		}
	}
	// 如果重新设定图层的高度大于之前的高度，将old~height之间的图层下降一层
	else if (height > old) {
		if (old >= 0) {
			for (h = old; h < height; h++) {
				ctl->sheets[h] = ctl->sheets[h + 1];
				ctl->sheets[h]->height = h;
			}
			ctl->sheets[height] = sht;
		}
			// 如果之前是隐藏状态，重新设定以为显示状态
		else {
			for (h = ctl->top; h >= height; h--) {
				ctl->sheets[h + 1] = ctl->sheets[h];
				ctl->sheets[h + 1]->height = h + 1;
			}
			ctl->sheets[height] = sht;
			ctl->top++;
		}
		sheet_refreshmap(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height);
		sheet_refreshsub(ctl, sht->vx0, sht->vy0, sht->vx0 + sht->bxsize, sht->vy0 + sht->bysize, height, height);
	}
	return;
}

/**
 * 刷新图层
 * @param sht 图层
 * @param bx0 x轴坐标
 * @param by0 y轴坐标
 * @param bx1 x轴坐标
 * @param by1 y轴坐标
 */
void sheet_refresh(struct SHEET *sht, int bx0, int by0, int bx1, int by1) {
	if (sht->height >= 0) {
		sheet_refreshsub(sht->ctl, sht->vx0 + bx0, sht->vy0 + by0, sht->vx0 + bx1, sht->vy0 + by1, sht->height, sht->height);
	}
	return;
}

/**
 * 移动图层
 * @param sht 图层
 * @param vx0 x轴坐标
 * @param vy0 y轴坐标
 */
void sheet_slide(struct SHEET *sht, int vx0, int vy0) {
//	struct SHTCTL *ctl = sht->ctl;
	int old_vx0 = sht->vx0, old_vy0 = sht->vy0;
	sht->vx0 = vx0;
	sht->vy0 = vy0;
	// 如果图层是显示状态，则需要重新绘制
	if (sht->height >= 0) {
		// 绘制移动前的图层
		sheet_refreshmap(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0);
		sheet_refreshsub(sht->ctl, old_vx0, old_vy0, old_vx0 + sht->bxsize, old_vy0 + sht->bysize, 0, sht->height - 1);

		// 绘制移动后的图层
		sheet_refreshmap(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height);
		sheet_refreshsub(sht->ctl, vx0, vy0, vx0 + sht->bxsize, vy0 + sht->bysize, sht->height, sht->height);
	}
	return;
}

/**
 * 释放图层
 * @param sht 图层
 */
void sheet_free(struct SHEET *sht) {
	// 如果图层是显示状态，则需要先隐藏
	if (sht->height >= 0) {
		sheet_updown(sht, -1);
	}
	sht->flags = 0;
	return;
}

/**
 * 绘制在h0~h1图层并且在vx0~vx1,vy0~vy1之间的内容
 * @param ctl 图层控制结构体
 * @param vx0 x轴坐标
 * @param vy0 y轴坐标
 * @param vx1 x轴坐标
 * @param vy1 y轴坐标
 * @param h0  从这个高度开始
 * @param h1  到这个高度结束
 */
void sheet_refreshsub(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0, int h1) {
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, *vram = ctl->vram, *map = ctl->map, sid;
	struct SHEET *sht;
	// 判断边界是否有超
	if (vx0 < 0) {
		vx0 = 0;
	}
	if (vy0 < 0) {
		vy0 = 0;
	}
	if (vx1 > ctl->xsize) {
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize) {
		vy1 = ctl->ysize;
	}
	// 自下向上绘制所有的图层
	for (h = h0; h <= h1; h++) {
		sht = ctl->sheets[h];
		buf = sht->buf;
		// 计算图层编号
		sid = sht - ctl->sheets0;

		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) {
			bx0 = 0;
		}
		if (by0 < 0) {
			by0 = 0;
		}
		if (bx1 > sht->bxsize) {
			bx1 = sht->bxsize;
		}
		if (by1 > sht->bysize) {
			by1 = sht->bysize;
		}

		// 只绘制h0到h1图层并且在vx0~vx1,vy0~vy1之间的内容
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (map[vy * ctl->xsize + vx] == sid) {
					vram[vy * ctl->xsize + vx] = buf[by * sht->bxsize + bx];
				}
			}
		}
	}
	return;
}

/**
 * 绘制在h0图层之上并且在vx0~vx1,vy0~vy1之间的内容
 * @param ctl 图层控制结构体
 * @param vx0 x轴坐标
 * @param vy0 y轴坐标
 * @param vx1 x轴坐标
 * @param vy1 y轴坐标
 * @param h0  从这个高度开始
 */
void sheet_refreshmap(struct SHTCTL *ctl, int vx0, int vy0, int vx1, int vy1, int h0) {
	int h, bx, by, vx, vy, bx0, by0, bx1, by1;
	unsigned char *buf, sid, *map = ctl->map;
	struct SHEET *sht;
	// 判断边界是否有超
	if (vx0 < 0) {
		vx0 = 0;
	}
	if (vy0 < 0) {
		vy0 = 0;
	}
	if (vx1 > ctl->xsize) {
		vx1 = ctl->xsize;
	}
	if (vy1 > ctl->ysize) {
		vy1 = ctl->ysize;
	}
	// 自下向上绘制所有的图层
	for (h = h0; h <= ctl->top; h++) {
		sht = ctl->sheets[h];
		// 计算图层编号
		sid = sht - ctl->sheets0;
		buf = sht->buf;
		bx0 = vx0 - sht->vx0;
		by0 = vy0 - sht->vy0;
		bx1 = vx1 - sht->vx0;
		by1 = vy1 - sht->vy0;
		if (bx0 < 0) {
			bx0 = 0;
		}
		if (by0 < 0) {
			by0 = 0;
		}
		if (bx1 > sht->bxsize) {
			bx1 = sht->bxsize;
		}
		if (by1 > sht->bysize) {
			by1 = sht->bysize;
		}

		// 只绘制所有图层在vx0~vx1,vy0~vy1之间的内容
		for (by = by0; by < by1; by++) {
			vy = sht->vy0 + by;
			for (bx = bx0; bx < bx1; bx++) {
				vx = sht->vx0 + bx;
				if (buf[by * sht->bxsize + bx] != sht->col_inv) {
					map[vy * ctl->xsize + vx] = sid;
				}
			}
		}
	}
	return;
}
