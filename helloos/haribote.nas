; haribote-os
; TAB=4

CYLS    EQU 0x0FF0                  ; 设定启动区
LEDS    EQU 0x0ff1
VMODE   EQU 0x0ff2                  ; 关于颜色数目的信息，颜色的位数
SCRNX   EQU 0x0ff4                  ; 屏幕的x分辨率
SCRNY   EQU 0x0ff6                  ; 屏幕的y分辨率
VRAM    EQU 0x0ff8                  ; 显存的起始地址

ORG 0xc200                          ; 程序加载地址
MOV AL, 0x13                        ; VGA显卡，320x200x8bit
MOV AH, 0x00
INT 0x10
MOV BYTE [VMODE], 8                 ; 记录画面模式
MOV WORD [SCRNX], 320
MOV WORD [SCRNY], 200
MOV DWORD [VRAM], 0x000a0000

; 用BIOS获得键盘上各种LED指示灯的状态
MOV AH, 0x02
INT 0x16
MOV [LEDS], AL
fin:
    HLT
    JMP fin