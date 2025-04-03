; 生成对象文件的模式
[FORMAT "WCOFF"]
; 486的指令集
[INSTRSET "i486p"]
; 32位模式
[BITS 32]
; 制作目标文件的文件名
[FILE "a_nask.nas"]

    GLOBAL _api_putchar, _api_end

[SECTION .text]
_api_putchar:       ; void api_putchar(char c)
    MOV     EDX, 1
    MOV     AL, [ESP + 4]
    INT     0x40
    RET

; 结束应用程序
_api_end:
    MOV     EDX, 4
    INT     0x40