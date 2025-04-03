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
	GLOBAL _asm_inthandler0d, _asm_inthandler20, _asm_inthandler21, _asm_inthandler27, _asm_inthandler2c
	GLOBAL _load_cr0, _store_cr0, _memtest_sub
	GLOBAL _load_tr, _farjmp, _asm_cons_putchar
	GLOBAL _farcall, _asm_hrb_api, _start_app
	EXTERN	_inthandler0d, _inthandler20, _inthandler21
    EXTERN	_inthandler27, _inthandler2c
    EXTERN	_cons_putchar, _hrb_api

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

; 一般异常中断处理程序
_asm_inthandler0d:
	STI
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		AX,SS
	CMP     AX, 1 * 8
	JNE     .from_app
	MOV     EAX, ESP
	; 保存中断时的SS
	PUSH    SS
	; 保存中断时的ESP
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler0d
	ADD     ESP, 8
	POPAD
	POP		DS
	POP		ES
	; 在INT 0x0d中需要这句
	ADD		ESP, 4
	IRETD

.from_app:
	CLI
    MOV     EAX, 1 * 8
    ; 将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统的ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -8
    ; 中断时的SS
    MOV     [ECX + 4], SS
    ; 中断时的ESP
    MOV     [ECX], ESP
    MOV     SS, AX
    MOV     ES, AX
    MOV     ESP, ECX
    STI
    CALL    _inthandler0d
    CTI
    CMP		EAX, 0
    JNE		.kill
    POP     ECX
    POP     EAX
    ; 将SS设回应用程序使用
    MOV     SS, AX
    ; 将ESP设回应用程序使用
    MOV     ESP, ECX
    POPAD
    POP     DS
    POP     ES
    ADD		ESP, 4
    IRETD

; 强制结束应用程序
.kill:
	MOV		EAX, 1 * 8
	MOV		ES, AX
	MOV		SS, AX
	MOV		DS, AX
	MOV		FS, AX
	MOV		GS, AX
	; 将栈恢复到start_app时的样子
	MOV		ESP, [0xfe4]
	STI
	POPAD
	RET


; 计时器中断处理程序
_asm_inthandler20:          ; void inthandler20(int *esp);
	PUSH	ES
	PUSH	DS
	PUSHAD
	MOV		AX,SS
	CMP     AX, 1 * 8
	JNE     .from_app
	MOV     EAX, ESP
	PUSH    SS
	PUSH	EAX
	MOV		AX,SS
	MOV		DS,AX
	MOV		ES,AX
	CALL	_inthandler20
	ADD     ESP, 8
	POPAD
	POP		DS
	POP		ES
	IRETD

; 如果是应用程序调用的中断处理程序
.from_app:
    MOV     EAX, 1 * 8
    ; 将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统的ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -8
    ; 中断时的SS
    MOV     [ECX + 4], SS
    ; 中断时的ESP
    MOV     [ECX], ESP
    MOV     SS, AX
    MOV     ES, AX
    MOV     ESP, ECX
    CALL    _inthandler20
    POP     ECX
    POP     EAX
    ; 将SS设回应用程序使用
    MOV     SS, AX
    ; 将ESP设回应用程序使用
    MOV     ESP, ECX
    POPAD
    POP     DS
    POP     ES
    IRETD

; 键盘中断处理程序
_asm_inthandler21:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		AX,SS
    CMP		AX,1*8
    JNE		.from_app
    MOV		EAX,ESP
    PUSH	SS
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	_inthandler21
    ADD		ESP,8
    POPAD
    POP		DS
    POP		ES
    IRETD

.from_app:
    MOV     EAX, 1 * 8
    ; 将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统的ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -8
    ; 中断时的SS
    MOV     [ECX + 4], SS
    ; 中断时的ESP
    MOV     [ECX], ESP
    MOV     SS, AX
    MOV     ES, AX
    MOV     ESP, ECX
    CALL    _inthandler21
    POP     ECX
    POP     EAX
    ; 将SS设回应用程序使用
    MOV     SS, AX
    ; 将ESP设回应用程序使用
    MOV     ESP, ECX
    POPAD
    POP     DS
    POP     ES
    IRETD

_asm_inthandler27:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		AX,SS
    CMP		AX,1*8
    JNE		.from_app
    MOV		EAX,ESP
    PUSH	SS
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	_inthandler27
    ADD		ESP,8
    POPAD
    POP		DS
    POP		ES
    IRETD

.from_app:
    MOV     EAX, 1 * 8
    ; 将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统的ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -8
    ; 中断时的SS
    MOV     [ECX + 4], SS
    ; 中断时的ESP
    MOV     [ECX], ESP
    MOV     SS, AX
    MOV     ES, AX
    MOV     ESP, ECX
    CALL    _inthandler27
    POP     ECX
    POP     EAX
    ; 将SS设回应用程序使用
    MOV     SS, AX
    ; 将ESP设回应用程序使用
    MOV     ESP, ECX
    POPAD
    POP     DS
    POP     ES
    IRETD

; 鼠标中断处理程序
_asm_inthandler2c:
    PUSH	ES
    PUSH	DS
    PUSHAD
    MOV		AX,SS
    CMP		AX,1*8
    JNE		.from_app
    MOV		EAX,ESP
    PUSH	SS
    PUSH	EAX
    MOV		AX,SS
    MOV		DS,AX
    MOV		ES,AX
    CALL	_inthandler2c
    ADD		ESP,8
    POPAD
    POP		DS
    POP		ES
    IRETD

.from_app:
    MOV     EAX, 1 * 8
    ; 将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统的ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -8
    ; 中断时的SS
    MOV     [ECX + 4], SS
    ; 中断时的ESP
    MOV     [ECX], ESP
    MOV     SS, AX
    MOV     ES, AX
    MOV     ESP, ECX
    CALL    _inthandler2c
    POP     ECX
    POP     EAX
    ; 将SS设回应用程序使用
    MOV     SS, AX
    ; 将ESP设回应用程序使用
    MOV     ESP, ECX
    POPAD
    POP     DS
    POP     ES
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
_farjmp:                        ; void farjmp(int eip, int cs);
        JMP     FAR [ESP + 4]   ; eip, cs
        ; 任务切换回来的时候要知道现在要返回的函数是哪个
        RET

; 显示单个字节API，应用程序将调用这个函数，然后这个函数调用cons_putchar函数
_asm_cons_putchar:
    ; 允许中断
    STI
    ; move
    ; 保存寄存器的值
    PUSHAD
    PUSH    1
    ; 将EAX高16位置为0，保留低16位，即AX
    AND     EAX, 0xff
    ; 字节数据
    PUSH    EAX
    ; cons地址
    PUSH    DWORD [0x0fec]
    ; 跳转到cons_putchar函数
    CALL    _cons_putchar
    ; 移除栈里的数据
    ADD     ESP, 12
    ; 恢复寄存器的值
    POPAD
    ; 如果用中断，将要用IRETD返回
    IRETD

; 这个函数是为了让汇编程序调用C函数
; 这个函数的参数是一个32位的地址
; 这个函数会将这个地址压入栈中，然后调用C函数
_farcall:           ; void farcall(int eip, int cs);
    CALL    FAR [ESP + 4]     ; eip, cs
    RET

;
_asm_hrb_api:

    PUSH    DS
    PUSH    ES
    ; 用于保存寄存器的值
    PUSHAD
    MOV     EAX, 1 * 8
    ; 先将DS设定为操作系统使用
    MOV     DS, AX
    ; 操作系统ESP
    MOV     ECX, [0xfe4]
    ADD     ECX, -40
    ; 保存应用程序的ESP
    MOV     [ECX + 32], ESP
    ; 保存应用程序的SS
    MOV     [ECX + 36], SS

; 将PUSHAD的值复制到系统栈

    MOV     EDX, [ESP]
    MOV     EBX, [ESP + 4]
    MOV     [ECX], EDX
    MOV     [ECX + 4], EBX
    MOV     EDX, [ESP + 8]
    MOV     EBX, [ESP + 12]
    MOV     [ECX + 8], EDX
    MOV     [ECX + 12], EBX
    MOV     EDX, [ESP + 16]
    MOV     EBX, [ESP + 20]
    MOV     [ECX + 16], EDX
    MOV     [ECX + 20], EBX
    MOV     EDX, [ESP + 24]
    MOV     EBX, [ESP + 28]
    MOV     [ECX + 24], EDX
    MOV     [ECX + 28], EBX
    ; 将剩余的段寄存器也设置给操作系统使用
    MOV     ES, AX
    MOV     SS, AX
    MOV     ESP, ECX
    ; 恢复中断
    STI
    ; 调用系统API
    CALL    _hrb_api

    ; 恢复应用程序的栈
    MOV     ECX, [ESP + 32]
    MOV     EAX, [ESP + 36]
    ; 禁止中断
    CLI
    MOV     SS, AX
    MOV     ESP, ECX
    POPAD
    POP     ES
    POP     DS
    ; 这个命令会自动执行STI
    IRETD

; 切换到应用程序，eip，cs，esp，ds为应用程序的参数
_start_app:         ; void start_app(int eip, int cs, int esp, int ds)
    ; 压入全部的寄存器
    PUSHAD
    ; 程序在运行之前会压入四个参数，分别为eip，cs，esp，ds，PUSHAD会压入32个字节的寄存器，esp会减4再减32，所以esp+36相当于eip参数，+8相当于cs参数
    ; 应用程序EIP
    MOV     EAX, [ESP + 36]
    ; 应用程序CS
    MOV     ECX, [ESP + 40]
    ; 应用程序ESP
    MOV     EDX, [ESP + 44]
    ; 应用程序DS/SS
    MOV     EBX, [ESP + 48]
    ; 操作系统现在的ESP，保存到0xfe4
    MOV     [0xfe4], ESP
    ; 切换过程中禁止使用中断
    CLI
    ; 将bx的值赋给段寄存器，主要为es与ds，其余的ss，fs，gs都只是保险起见
    MOV     ES, BX
    MOV     SS, BX
    MOV     DS, BX
    MOV     FS, BX
    MOV     GS, BX
    ; 栈指针
    MOV     ESP, EDX
    ; 可以使用中断
    STI
    ; 用于切换的cs
    PUSH    ECX
    ; 用于切换的eip
    PUSH    EAX
    ; 调用应用程序
    CALL    FAR [ESP]

; 应用程序结束以后返回这个地方
    MOV     EAX, 1 * 8
    ; 切换回操作系统时禁止中断
    CLI
    MOV     ES, AX
    MOV     SS, AX
    MOV     DS, AX
    MOV     FS, AX
    MOV     GS, AX
    MOV     ESP, [0xfe4]
    STI
    POPAD
    RET


