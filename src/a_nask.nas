; 生成对象文件的模式
[FORMAT "WCOFF"]
; 486的指令集
[INSTRSET "i486p"]
; 32位模式
[BITS 32]
; 制作目标文件的文件名
[FILE "a_nask.nas"]

    GLOBAL _api_putchar, _api_end, _api_putstr, _api_openwin, _api_putstrwin, _api_boxfilwin

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

; 显示窗口
_api_openwin:
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     EDX, 5
    MOV     EBX, [ESP + 16]
    MOV     ESI, [ESP + 20]
    MOV     EDI, [ESP + 24]
    MOV     EAX, [ESP + 28]
    MOV     ECX, [ESP + 32]
    INT     0x40
    POP     EBX
    POP     ESi
    POP     EDI
    RET

;
_api_putstrwin:
    PUSH    EDI
    PUSH    ESI
    PUSH    EBP
    PUSH    EBX
    MOV     EDX, 6
    MOV     EBX, [ESP + 20]
    MOV     ESI, [ESP + 24]
    MOV     EDI, [ESP + 28]
    MOV     EAX, [ESP + 32]
    MOV     ECX, [ESP + 36]
    MOV     EBP, [ESP + 40]
    INT     0x40
    POP     EBX
    POP     EBP
    POP     ESI
    POP     EDI
    RET


_api_boxfilwin:
    PUSH    EDI
    PUSH    ESI
    PUSH    EBP
    PUSH    EBX
    MOV     EDX, 7
    MOV     EBX, [ESP + 20]
    MOV     EAX, [ESP + 24]
    MOV     ECX, [ESP + 28]
    MOV     ESI, [ESP + 32]
    MOV     EDI, [ESP + 36]
    MOV     EBP, [ESP + 40]
    INT     0x40
    POP     EBX
    POP     EBP
    POP     ESI
    POP     EDI
    RET
