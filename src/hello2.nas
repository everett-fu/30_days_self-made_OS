[INSTRSET "i486p"]
[BITS 32]
    MOV     EDX, 2
    MOV     EBX, msg
    INT     0x40
    ; 结束应用程序
    MOV     EDX, 4
    INT     0x40
msg:
    DB  "hello2", 0