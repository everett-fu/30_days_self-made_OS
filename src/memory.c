/*
 * Filename: memory.c
 * Author: everett.fu
 * Created: 24-8-2
 * Last Modified: 24-8-2
 * Description:
 * 这个文件包含了内存管理的实现。
 *
 * Functions:
 * - memtest: 内存检查
 * - memman_init: 初始化内存管理
 * - memman_total: 统计剩余内存总大小
 * - memman_alloc: 分配内存
 * - memman_free: 释放内存
 * - memman_alloc_4k: 分配4K内存
 * - memman_free_4k: 释放4K内存
 *
 * Usage:
 */
#include "bootpack.h"


#define EFLAGS_AC_BIT 0x00040000
#define CR0_CACHE_DISABLE 0x60000000

/**
 * 内存检查
 * @param start		开始地址
 * @param end		结束地址
 * @return			返回内存检查结果
 */
unsigned int memtest(unsigned int start, unsigned int end) {
	char flg486 = 0;
	unsigned int eflg, cr0, i;

	// 确认CPU是386还是486以上的
	eflg = io_load_eflags();
	eflg |= EFLAGS_AC_BIT;
	io_store_eflags(eflg);
	eflg = io_load_eflags();

	// 如果是386，即使设定AC=1，AC的值还会自动回到0
	if ((eflg & EFLAGS_AC_BIT) != 0) {
		flg486 = 1;
	}

	eflg &= ~EFLAGS_AC_BIT;
	io_store_eflags(eflg);

	if (flg486 != 0) {
		cr0 = load_cr0();
		// 禁止缓存
		cr0 |= CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}

	i = memtest_sub(start, end);

	if (flg486 != 0) {
		cr0 = load_cr0();
		// 允许缓存
		cr0 &= ~CR0_CACHE_DISABLE;
		store_cr0(cr0);
	}
	return i;
}

/**
 * 初始化内存管理
 * @param man		内存管理结构体
 */
void memman_init(struct MEMMAN *man) {
	man->frees = 0;
	man->maxfrees = 0;
	man->lostsize = 0;
	man->losts = 0;
	return;
}

/**
 * 统计剩余内存总大小
 * @param man		内存管理结构体
 * @return			返回剩余内存总大小
 */
unsigned int memman_total(struct MEMMAN *man) {
	unsigned int i, sum = 0;
	for (i = 0; i < man->frees; i++) {
		sum += man->free[i].size;
	}
	return sum;
}

/**
 * 分配内存
 * @param man		内存管理结构体
 * @param size		需要分配的内存大小
 * @return			返回分配的内存地址
 */
unsigned int memman_alloc(struct MEMMAN *man, unsigned int size) {
	unsigned int i, a;
	// 遍历所有空闲内存块
	for (i = 0; i < man->frees; i++) {
		// 找到一个大于size的内存块
		if (man->free[i].size >= size) {
			a = man->free[i].addr;
			man->free[i].addr += size;
			man->free[i].size -= size;
			// 如果需要的内存与空余的内存大小相等，则删除该内存块
			if (man->free[i].size == 0) {
				man->frees--;
				for (; i < man->frees; i++) {
					man->free[i] = man->free[i + 1];
				}
			}
			return a;
		}
	}
	return 0;
}

/**
 * 释放内存
 * @param man		内存管理结构体
 * @param addr		释放的内存地址
 * @param size		释放的内存大小
 * @return			返回0表示成功，返回-1表示失败
 */
int memman_free(struct MEMMAN *man, unsigned int addr, unsigned int size) {
	int i, j;
	for (i = 0; i < man->frees; i++) {
		if (man->free[i].addr > addr) {
			break;

		}
	}

	// free[i - 1].addr + free[i - 1].size == addr
	// 不是第一个
	if (i > 0) {
		// 是否可以与前面的合并
		if (man->free[i - 1].addr + man->free[i - 1].size == addr) {
			man->free[i - 1].size += size;
			// 不是最后一个
			if (i < man->frees) {
				// 是否可以与后面的合
				if (addr + size == man->free[i].addr) {
					man->free[i - 1].size += man->free[i].size;
					man->frees--;
					for (; i < man->frees; i++) {
						man->free[i] = man->free[i + 1];
					}
				}
			}
			return 0;
		}
	}
	//不能与前面的合并
	if (i < man->frees) {
		// 判断是否能与后面的合并
		if (addr + size == man->free[i].addr) {
			man->free[i].addr = addr;
			man->free[i].size += size;
			return 0;
		}
	}
	//不能与前面和后面合并
	if (man->frees < MEMMAN_FREES) {
		// 将free[i]以后的，向后移动，腾出一块空间
		for (j = man->frees; j > i; j--) {
			man->free[j] = man->free[j - 1];
		}
		man->frees++;
		// 更新最大值
		if (man->maxfrees < man->frees) {
			man->maxfrees = man->frees;
		}
		man->free[i].addr = addr;
		man->free[i].size = size;
		return 0;
	}

	// 不能向后移动
	man->losts++;
	man->lostsize += size;
	return -1;
}

/**
 * 分配4k大小的内存
 * @param man		内存管理结构体
 * @param size		需要分配的内存大小
 * @return			返回分配的内存地址
 */
unsigned int memman_alloc_4k(struct MEMMAN *man, unsigned int size) {
	unsigned int addr;
	size = (size + 0xfff) & 0xfffff000;
	addr = memman_alloc(man, size);
	return addr;
}

/**
 * 释放4k大小的内存
 * @param man		内存管理结构体
 * @param addr		释放的内存地址
 * @param size		释放的内存大小
 * @return			返回0表示成功，返回-1表示失败
 */
int memman_free_4k(struct MEMMAN *man, unsigned int addr, unsigned int size) {
	int i;
	size = (size + 0xfff) & 0xfffff000;
	i = memman_free(man, addr, size);
	return i;
}
