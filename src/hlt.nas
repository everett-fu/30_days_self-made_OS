[BITS 32]
    MOV     AL, 'A'
    CALL    2*8:0xb65
fin:
    HLT
    JMP fin