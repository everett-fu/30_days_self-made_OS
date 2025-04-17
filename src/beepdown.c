/*
 * Filename: beepdown.c
 * Author: everett.fu
 * Created: 25-4-17
 * Last Modified: 25-4-17
 * Description:
 * 这个文件是
 *
 *Functions:
 * - test: 这是个测试函数
 *
 * Usage:
 */
void api_end(void);
int api_getkey(int mode);
int api_alloctimer(void);
void api_inittimer(int timer, int data);
void api_settimer(int timer, int time);
void api_beep(int tone);

void HariMain(void) {
	int i, timer;
	timer = api_alloctimer();
	api_inittimer(timer, 128);
	for (i = 20000000; i >= 20000; i -= i / 100) {
		api_beep(i);
		api_settimer(timer, 1);
		if (api_getkey(1) != 128) {
			break;
		}
	}
	api_beep(0);
	api_end();

}
