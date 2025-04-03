[INSTRSET "i486p"]
[BITS 32]
	; 操作系统段号
	MOV		EAX, 1 * 8
	MOV		DS, AX
	MOV		BYTE [0x102600], 0
	; 结束应用程序
	MOV     EDX, 4
	INT     0x40
