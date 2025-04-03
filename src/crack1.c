/*
 * Filename: crack1.c
 * Author: everett.fu
 * Created: 25-4-1
 * Last Modified: 25-4-1
 * Description:
 * 这个文件是一个恶意漏洞，用来测试系统的健壮性
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
void api_end(void);

void HariMain(void) {
	*((char *)0x00102600) = 0;
	api_end();
}
