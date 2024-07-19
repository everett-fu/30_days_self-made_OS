; naskfunc
; TAB = 4

[FORMAT "WCOFF"]										; 制作目标文件的模式
[INSTRSET "i486p"]										; 486的指令集
[BITS 32]												; 制作32位模式的目标文件

; 制作目标文件的信息
[FILE "naskfunc.nas"]									; 制作目标文件的文件名
; 程序中包含的函数名
	GLOBAL _io_hlt, _io_cli, _io_sti, _io_stihlt
	GLOBAL _io_in8, _io_in16, _io_in32
	GLOBAL _io_out8, _io_out16, _io_out32
	GLOBAL _io_load_eflags, _io_store_eflags
	GLOBAL	_load_gdtr, _load_idtr
	GLOBAL	_asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	EXTERN	_inthandler21, _inthandler27, _inthandler2c

; 函数定义
[SECTION .text]

; 休眠函数
_io_hlt:												; void io_hlt(void);
	HLT													; 休眠
	RET													; 返回

; 将中断标志置为0
_io_cli:												; void io_cli(void);
		CLI
		RET

; 将中断标志置为1
_io_sti:												; void io_sti(void);
		STI
		RET

_io_stihlt:												; void io_stihlt(void);
		STI
		HLT
		RET

; 从端口中读取8位
_io_in8:												; int io_in8(int port);
		MOV EDX,[ESP+4]		; port
		MOV EAX,0
		IN  AL,DX
		RET

; 从端口中读取16位
_io_in16:												; int io_in16(int port);
		MOV EDX,[ESP+4]		; port
		MOV EAX,0
		IN  AX,DX
		RET

; 从端口中读取32位
_io_in32:												; int io_in32(int port);
		MOV EDX,[ESP+4]		; port
		IN  EAX,DX
		RET

; 从端口中输出8位
_io_out8:												; void io_out8(int port, int data);
		MOV EDX,[ESP+4]		; port
		MOV AL,[ESP+8]		; data
		OUT DX,AL
		RET

; 从端口中输出16位
_io_out16:												; void io_out16(int port, int data);
		MOV EDX,[ESP+4]		; port
		MOV EAX,[ESP+8]		; data
		OUT DX,AX
		RET

; 从端口中输出32位
_io_out32:												; void io_out32(int port, int data);
		MOV EDX,[ESP+4]		; port
		MOV EAX,[ESP+8]		; data
		OUT DX,EAX
		RET

; 加载标志寄存器
_io_load_eflags:										; int io_load_eflags(void);
		PUSHFD											; PUSH EFLAGS という意味
		POP EAX
		RET

; 写入标志寄存器
_io_store_eflags:										; void io_store_eflags(int eflags);
		MOV EAX,[ESP+4]
		PUSH EAX
		POPFD											; POP EFLAGS という意味
		RET												; 返回

; 给GDTR寄存器赋值
_load_gdtr:												; void load_gdtr(int limit, int addr);
		MOV AX,[ESP+4]									; limit
		MOV [ESP+6],AX
		LGDT [ESP+6]
		RET

; 给IDTR寄存器赋值
_load_idtr:												; void load_idtr(int limit, int addr);
		MOV AX,[ESP+4]									; limit
		MOV [ESP+6],AX
		LIDT [ESP+6]
		RET

; 键盘中断处理程序
_asm_inthandler21:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler21
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

_asm_inthandler27:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler27
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD

; 鼠标中断处理程序
_asm_inthandler2c:
		PUSH	ES
		PUSH	DS
		PUSHAD
		MOV		EAX,ESP
		PUSH	EAX
		MOV		AX,SS
		MOV		DS,AX
		MOV		ES,AX
		CALL	_inthandler2c
		POP		EAX
		POPAD
		POP		DS
		POP		ES
		IRETD