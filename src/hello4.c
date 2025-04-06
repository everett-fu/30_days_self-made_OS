/*
 * Filename: hello4.c
 * Author: everett.fu
 * Created: 25-4-6
 * Last Modified: 25-4-6
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */

void api_putstr(char *str);
void api_end(void);

void HariMain(void) {
	api_putstr("hello world\n");
	api_end();
}