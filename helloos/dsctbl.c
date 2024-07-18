/*
 * Filename: dsctbl.c
 * Author: everett.fu
 * Created: 24-7-18
 * Last Modified: 24-7-18
 * Description:
 * 这个文件包含了一些关于GDT,IDT的函数。
 *
 * Functions:
 * - init_gdtidt: 初始化GDT,IDT
 * - set_segmdesc: 设置段描述符
 * - set_gatedesc: 设置门描述符
 * - load_gdtr: 加载GDT
 * - load_idtr: 加载IDT
 *
 * Usage:
 */

# include "bootpack.h"

/**
 * 初始化GDT,IDT
 */
void init_gdtidt(void) {
	struct SEGMENT_DESCRIPTOR *gdt = (struct SEGMENT_DESCRIPTOR *) 0x00270000;
	struct GATE_DESCRIPTOR *idt = (struct GATE_DESCRIPTOR *) 0x0026f800;
	int i;

	// GDT初始化
	for (i = 0; i < 8192; i++) {
		set_segmdesc(gdt + i, 0, 0, 0);
	}
	set_segmdesc(gdt + 1, 0xffffffff, 0x00000000, 0x4092);
	set_segmdesc(gdt + 2, 0x0007ffff, 0x00280000, 0x409a);
	load_gdtr(0xffff, 0x00270000);

	// IDT初始化
	for (i = 0; i < 256; i++) {
		set_gatedesc(idt + i, 0, 0, 0);
	}
	load_idtr(0x7ff, 0x0026f800);

	return;
}

/**
 * 设置段描述符
 * @param sd		段描述符
 * @param limit		段限长
 * @param base		段基址
 * @param ar		访问权限
 */
void set_segmdesc(struct SEGMENT_DESCRIPTOR *sd, unsigned int limit, int base, int ar) {
	if (limit > 0xfffff) {
		ar |= 0x8000; /* G_bit = 1 */
		limit /= 0x1000;
	}
	sd->limit_low = limit & 0xffff;
	sd->base_low = base & 0xffff;
	sd->base_mid = (base >> 16) & 0xff;
	sd->access_right = ar & 0xff;
	sd->limit_high = ((limit >> 16) & 0x0f) | ((ar >> 8) & 0xf0);
	sd->base_high = (base >> 24) & 0xff;
	return;
}

/**
 * 设置门描述符
 * @param gd		门描述符
 * @param offset	偏移量
 * @param selector	段选择子
 * @param ar		访问权限
 */
void set_gatedesc(struct GATE_DESCRIPTOR *gd, int offset, int selector, int ar) {
	gd->offset_low = offset & 0xffff;
	gd->selector = selector;
	gd->dw_count = (ar >> 8) & 0xff;
	gd->access_right = ar & 0xff;
	gd->offset_high = (offset >> 16) & 0xffff;
	return;
}