/*
 * Filename: file.c
 * Author: everett.fu
 * Created: 25-3-25
 * Last Modified: 25-3-25
 * Description:
 * 这个文件是文件相关的文件，包含了文件的读取与加载函数。
 *
 *Functions:
 * - file_readfat: 读取fat表
 * - file_loadfile: 加载文件
 *
 * Usage:
 */

#include "bootpack.h"

/**
 * 对磁盘镜像中fat表进行解压
 */
void file_readfat(int *fat, unsigned char *img) {
	int i, j = 0;
	for (i = 0; i < 2880; i += 2) {
		fat[i + 0] = (img[j + 0] | (img[j + 1] << 8)) & 0xfff;
		fat[i + 1] = (img[j + 1] >> 4 | (img[j + 2] << 4)) & 0xfff;
		j += 3;
	}
	return;
}

/**
 * 加载文件
 * @param clustno		文件的起始簇号
 * @param size			文件的大小
 * @param buf			文件的缓冲区
 * @param fat			fat表
 * @param img			磁盘镜像
 */
void file_loadfile(int clustno, int size, char *buf, int *fat, char *img) {
	int i;
	for (;;) {
		// 如果文件的大小小于512字节
		if(size <= 512) {
			for (i = 0; i < size; i++) {
				buf[i] = img[clustno * 512 + i];
			}
			break;
		}
		// 如果文件的大小大于512字节
		for (i = 0; i < 512; i++) {
			buf[i] = img[clustno * 512 + i];
		}
		size -= 512;
		buf += 512;
		clustno = fat[clustno];
	}
	return;
}

/**
 * 查找文件
 * @param name		文件名
 * @param finfo		文件信息
 * @param max		总文件数量
 */
struct FILEINFO * file_search(char *name, struct FILEINFO *finfo, int max) {
	int x, y;
	char s[12];
	// 将字符串S用空格填充
	for (y = 0; y < 11; y++) {
		s[y] = ' ';
	}

	// 将文件名读取到字符串S中，文件名8位，不满的用空格填充，第8位开始为后缀名，后缀名3位
	for (y = 0, x = 0; name[x] != 0; x++) {
		// 文件名不可能大于等于11
		if (y >= 11) {
			// 没有找到
			return 0;
		}
		if (name[x] == '.' && y <= 8) {
			y = 8;
		}
		else {
			s[y] = name[x];
			// 如果是小写字母，将字母转换成大写
			if (s[y] >= 'a' && s[y] <= 'z') {
				s[y] -= 0x20;
			}
			y++;
		}
	}
	// 查找文件
	// 与文件格式中一个个对比
	for (x = 0; x < max; x++) {
		// 该数据段不含内容
		if (finfo[x].name[0] == 0x00) {
			break;
		}
		// 是否找到文件，为0则是没有找到，为1则找到了文件
		char flag = 1;
		// 为不是目录或者归档文件
		if ((finfo[x].type & 0x18) == 0) {
			// 文件名不能匹配上
			for (y = 0; y < 11; y++) {
				if (finfo[x].name[y] != s[y]) {
					flag = 0;
					break;
				}
			}
			// 可以匹配上，跳出循环，开始显示字符
			if (flag == 1) {
				return finfo + x;
			}
		}
	}
	return 0;
}
