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
	GLOBAL _load_gdtr, _load_idtr
	GLOBAL _asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	EXTERN _inthandler20, _inthandler21, _inthandler27, _inthandler2c
	GLOBAL _load_cr0, _store_cr0, _memtest_sub
	GLOBAL _load_tr, _taskswitch4

; 函数定义
[SECTION .text]

; 休眠函数
_io_hlt:												; void io_hlt(void);
	HLT													; 休眠
	RET													; 返回

; 将中断标志置为0，屏蔽中断
_io_cli:												; void io_cli(void);
	CLI
	RET

; 将中断标志置为1，允许中断
_io_sti:												; void io_sti(void);
	STI
	RET

; 将中断标志置为1并休眠
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

; 计时器中断处理程序
_asm_inthandler20:
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		EAX,ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler20
	POP		EAX
	POPAD
	POP		DS
	POP		ES
	IRETD

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

; 加载CR0寄存器
_load_cr0:						; int load_cr0(void);
	MOV EAX, CR0
	RET

; 写入CR0寄存器
_store_cr0:						; void store_cr0(int cr0);
	MOV EAX, [ESP+4]
	MOV CR0, EAX
	RET

; 内存检查
_memtest_sub:	; unsigned int memtest_sub(unsigned int start, unsigned int end)
		PUSH	EDI
		PUSH	ESI
		PUSH	EBX
		MOV		ESI,0xaa55aa55			; pat0 = 0xaa55aa55;
		MOV		EDI,0x55aa55aa			; pat1 = 0x55aa55aa;
		MOV		EAX,[ESP+12+4]			; i = start;
mts_loop:
		MOV		EBX,EAX
		ADD		EBX,0xffc				; p = i + 0xffc;
		MOV		EDX,[EBX]				; old = *p;
		MOV		[EBX],ESI				; *p = pat0;
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		EDI,[EBX]				; if (*p != pat1) goto fin;
		JNE		mts_fin
		XOR		DWORD [EBX],0xffffffff	; *p ^= 0xffffffff;
		CMP		ESI,[EBX]				; if (*p != pat0) goto fin;
		JNE		mts_fin
		MOV		[EBX],EDX				; *p = old;
		ADD		EAX,0x1000				; i += 0x1000;
		CMP		EAX,[ESP+12+8]			; if (i <= end) goto mts_loop;
		JBE		mts_loop
		POP		EBX
		POP		ESI
		POP		EDI
		RET
mts_fin:
		MOV		[EBX],EDX				; *p = old;
		POP		EBX
		POP		ESI
		POP		EDI
		RET

; 给tr寄存器赋值
_load_tr:                       ; void load_tr(int tr);
        LTR     [ESP + 4]
        RET

; far模式跳转
; near与far模式跳转
; near只改变ip
; far模式同时改变cs与ip
_taskswitch4:                   ; void taskswitch4(void);
        JMP     4 * 8:0
        ; 任务切换回来的时候要知道现在要返回的函数是哪个
        RET