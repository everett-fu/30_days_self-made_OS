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
