[FORMAT "WCOFF"]
[INSTRSET "i486p"]
[BITS 32]
[FILE "hello.nas"]
    GLOBAL  _HariMain

[SECTION .text]
_HariMain:
    MOV     EDX, 1
    MOV     ECX, msg
putloop:
    MOV     AL, [ECX]
    CMP     AL, 0
    JE      fin
    INT     0x40
    ADD     ECX, 1
    JMP     putloop
fin:
    MOV     EDX, 4
    INT     0x40
[SECTION .data]
msg:
    DB "hello", 0