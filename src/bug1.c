/*
 * Filename: bug1.c
 * Author: everett.fu
 * Created: 25-4-4
 * Last Modified: 25-4-4
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
void api_putchar(char c);
void api_end(void);

void HariMain(void) {
	char a[100];
	a[10] = 'A';
	api_putchar(a[10]);
	a[102] = 'B';
	api_putchar(a[102]);
	a[123] = 'C';
	api_putchar(a[123]);
	api_end();
}
