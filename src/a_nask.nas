; 生成对象文件的模式
[FORMAT "WCOFF"]
; 486的指令集
[INSTRSET "i486p"]
; 32位模式
[BITS 32]
; 制作目标文件的文件名
[FILE "a_nask.nas"]

    GLOBAL _api_putchar, _api_end, _api_putstr

[SECTION .text]
; 显示字符
_api_putchar:       ; void api_putchar(char c)
    MOV     EDX, 1
    MOV     AL, [ESP + 4]
    INT     0x40
    RET

; 结束应用程序
_api_end:
    MOV     EDX, 4
    INT     0x40

; 显示字符串
_api_putstr:        ; void api_putstr(char *str)
    PUSH    EBX
    MOV     EDX, 2
    MOV     EBX, [ESP + 8]
    INT     0x40
    POP     EBX
    RET