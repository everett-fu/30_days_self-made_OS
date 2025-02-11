; haribote-os boot asm
; TAB=4

BOTPAK	EQU		0x00280000		; bootpackのロード先
DSKCAC	EQU		0x00100000		; ディスクキャッシュの場所
DSKCAC0	EQU		0x00008000		; ディスクキャッシュの場所（リアルモード）

; BOOT_INFO信息
CYLS    EQU 0x0ff0                  ; 设定启动区
LEDS    EQU 0x0ff1
VMODE   EQU 0x0ff2                  ; 关于颜色数目的信息，颜色的位数
SCRNX   EQU 0x0ff4                  ; 屏幕的x分辨率
SCRNY   EQU 0x0ff6                  ; 屏幕的y分辨率
VRAM    EQU 0x0ff8                  ; 显存的起始地址

ORG 0xc200                          ; 程序加载地址

; 画面显示设定
MOV BX, 0x4105                        ; VGA显卡，320x200x8bit
MOV AX, 0x4f02
INT 0x10
MOV BYTE [VMODE], 8                 ; 记录画面模式
MOV WORD [SCRNX], 1024
MOV WORD [SCRNY], 768
MOV DWORD [VRAM], 0xe0000000

; 用BIOS获得键盘上各种LED指示灯的状态
MOV AH, 0x02
INT 0x16
MOV [LEDS], AL

; PIC关闭一切中断
; 由于AT兼容机的规格，PIC的初始化必须在CLI之前进行，否则有时会挂起
; PIC的初始化稍后再做
		MOV		AL,0xff
		OUT		0x21,AL
		; NOP指令使CPU什么都不做，只是让CPU等待一段时间
		NOP						; 如果连续执行OUT指令，有些机器会无法正常运行
		OUT		0xa1,AL

		CLI						; 禁止CPU处理中断

; A20线开启
		CALL	waitkbdout
		MOV		AL,0xd1
		OUT		0x64,AL
		CALL	waitkbdout
		MOV		AL,0xdf			; enable A20
		OUT		0x60,AL
		CALL	waitkbdout

; 切换到保护模式
[INSTRSET "i486p"]				; 使用486指令集

		LGDT	[GDTR0]			; 临时设定GDT
		MOV		EAX,CR0
		AND		EAX,0x7fffffff	; 设置bit31为0（为了禁止分页）
		OR		EAX,0x00000001	; 设置bit0为1（为了进入保护模式）
		MOV		CR0,EAX
		JMP		pipelineflush
pipelineflush:
		MOV		AX,1*8			; 可读写的段 32bit
		MOV		DS,AX
		MOV		ES,AX
		MOV		FS,AX
		MOV		GS,AX
		MOV		SS,AX

; bootpack的转送
		MOV		ESI,bootpack	; 转送源地址
		MOV		EDI,BOTPAK		; 转送目的地址
		MOV		ECX,512*1024/4
		CALL	memcpy

; 磁盘数据最终转送到它本来的位置去
; 从启动扇区开始
		MOV		ESI,0x7c00		; 转送源地址
		MOV		EDI,DSKCAC		; 转送目的地址
		MOV		ECX,512/4
		CALL	memcpy

; 剩余的其他扇区
		MOV		ESI,DSKCAC0+512	; 转送源地址
		MOV		EDI,DSKCAC+512	; 转送目的地址
		MOV		ECX,0
		MOV		CL,BYTE [CYLS]
		IMUL	ECX,512*18*2/4	; 从柱面变换为字节数/4
		SUB		ECX,512/4		; 减去IPL
		CALL	memcpy

; 必须由asmhead来完成的工作已经全部完毕
; 后面由bootpack来完成
; bootpack启动
		MOV		EBX,BOTPAK
		MOV		ECX,[EBX+16]
		ADD		ECX,3			; ECX += 3;
		SHR		ECX,2			; ECX /= 4;
		JZ		skip			; 没有要转送的东西
		MOV		ESI,[EBX+20]	; 转送的源地址
		ADD		ESI,EBX
		MOV		EDI,[EBX+12]	; 转送的目的地址
		CALL	memcpy
skip:
		MOV		ESP,[EBX+12]	; 栈初始值
		JMP		DWORD 2*8:0x0000001b

waitkbdout:
		IN		 AL,0x64
		AND		 AL,0x02
		IN		 AL,0x60
		JNZ		waitkbdout		; AND命令的结果不为0的话，就跳到waitkbdout
		RET

memcpy:
		MOV		EAX,[ESI]
		ADD		ESI,4
		MOV		[EDI],EAX
		ADD		EDI,4
		SUB		ECX,1
		JNZ		memcpy			; 减法运算的结果如果不是0，就跳到memcpy
		RET
; memcpyはアドレスサイズプリフィクスを入れ忘れなければ、ストリング命令でも書ける

; ALIGNB一直添加0，直到地址是16的倍数
		ALIGNB	16
GDT0:
		RESB	8				; NULL selector
		DW		0xffff,0x0000,0x9200,0x00cf	; 可以读写的段(segment)32bit
		DW		0xffff,0x0000,0x9a28,0x0047	; 可以执行的段(segment)32bit(bootpack用)

		DW		0
GDTR0:
		DW		8*3-1
		DD		GDT0

		ALIGNB	16
bootpack: