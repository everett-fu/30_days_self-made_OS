[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello.nas"]
    GLOBAL  _HariMain

[SECTION .text]
_HariMain:
    MOV     EDX, 2
    MOV     EBX, msg
    INT     0x40
    ; 结束应用程序
    MOV     EDX, 4
    INT     0x40
[SECTION .data]
msg:
    DB  "hello2", 0