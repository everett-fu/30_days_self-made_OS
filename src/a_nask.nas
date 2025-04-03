; 生成对象文件的模式
[FORMAT "WCOFF"]
; 486的指令集
[INSTRSET "i486p"]
; 32位模式
[BITS 32]
; 制作目标文件的文件名
[FILE "a_nask.nas"]

    GLOBAL _api_putchar

[SECTION .text]
_api_putchar:
    MOV     EDX, 1
    MOV     AL, [ESP + 4]
    INT     0x40
    RET