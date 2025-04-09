; 生成对象文件的模式
[FORMAT "WCOFF"]
; 486的指令集
[INSTRSET "i486p"]
; 32位模式
[BITS 32]
; 制作目标文件的文件名
[FILE "a_nask.nas"]

    GLOBAL _api_putchar, _api_end, _api_putstr, _api_openwin, _api_putstrwin, _api_boxfilwin, _api_initmalloc
    GLOBAL _api_malloc, _api_free, _api_point

[SECTION .text]
; 显示字符
_api_putchar:       ; void api_putchar(char c);
    MOV     EDX, 1
    MOV     AL, [ESP + 4]
    INT     0x40
    RET

; 结束应用程序
_api_end:
    MOV     EDX, 4
    INT     0x40

; 显示字符串
_api_putstr:        ; void api_putstr(char *str);
    PUSH    EBX
    MOV     EDX, 2
    MOV     EBX, [ESP + 8]
    INT     0x40
    POP     EBX
    RET

; 显示窗口
_api_openwin:           ; int api_openwin(char *buf, int x, int y, int color, char *title);
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

; 窗口上显示字符api
_api_putstrwin:         ; void api_putstrwin(int win, int x, int y, int color, int len, char *str);
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

; 窗口上描绘方块api
_api_boxfilwin:         ; void api_boxfilwin(int win, int x, int y, int w, int h, int color);
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

; 应用程序栈初始化api
_api_initmalloc:        ; void api_initmalloc(void);
    PUSH    EBX
    MOV     EDX, 8
    ; malloc内存空间的地址
    MOV     EBX, [CS:0x0020]
    MOV     EAX, EBX
    ; 加上32kb，用于内存管理器的使用
    ADD     EAX, 32 * 1024
    ; 数据段的大小
    MOV     ECX, [CS:0x0000]
    SUB     ECX, EAX
    INT     0x040
    POP     EBX
    RET

; 应用程序栈分配api
_api_malloc:            ; char * api_malloc(int size);
    PUSH    EBX
    MOV     EDX, 9
    MOV     EBX, [CS:0x0020]
    MOV     ECX, [ESP + 8]
    INT     0x040
    POP     EBX
    RET

; 应用程序栈回收api
_api_free:              ; void api_free(char *addr, int size);
    PUSH    EBX
    MOV     EDX, 10
    MOV     EBX, [CS:0x0020]
    MOV     EAX, [ESP + 8]
    MOV     ECX, [ESP + 12]
    INT     0x040
    POP     EBX
    RET

; 画点api
_api_point:             ; void api_point(int win, int x, int y, int col);
    PUSH    EDI
    PUSH    ESI
    PUSH    EBX
    MOV     EDX, 11
    MOV     EBX, [ESP + 16]
    MOV     ESI, [ESP + 20]
    MOV     EDI, [ESP + 24]
    MOV     EAX, [ESP + 28]
    INT     0x40
    pop     EBX
    pop     ESI
    pop     EDI
    RET
