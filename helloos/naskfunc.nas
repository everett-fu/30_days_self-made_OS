; naskfunc
; TAB = 4

[FORMAT "WCOFF"]                            ; 制作目标文件的模式
[INSTRSET "i486p"]                          ; 486的指令集
[BITS 32]                                   ; 制作32位模式的目标文件

; 制作目标文件的信息
[FILE "naskfunc.nas"]                       ; 制作目标文件的文件名
    GLOBAL    _io_hlt                       ; 程序中包含的函数名

; 函数定义
[SECTION .text]
_io_hlt:                                    ; 等待中断的函数
    HLT                                     ; 休眠
    RET                                     ; 返回