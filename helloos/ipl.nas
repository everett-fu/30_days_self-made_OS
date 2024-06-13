; hello-os
; TAB = 4

ORG 0x7c00

; 以下这段是标准FAT12格式软盘专用的引导扇区代码
JMP entry
DB  0x90

DB  "HELLOIPL"                  ;启动区的名称可以是任意的字符串（8字符）
DW  512                         ;每扇区字节数（必须为512字节）
DB  1                           ;每簇扇区数（必须为1个扇区）
DW  1                           ;Boot占几个扇区（一般从第一个扇区开始）
DB  2                           ;FAT的数量（一般为2个）
DW  224                         ;根目录的最大容量（一般为224个条目）
DW  2880                        ;软盘的扇区总数（一般为2880个扇区）
DB  0xf0                        ;磁盘的种类（一般为0xf0）
DW  9                           ;每簇的扇区数（一般为9个扇区）
DW  18                          ;一个磁道有几个扇区（一般为18个）
DW  2                           ;磁头数（一般为2个）
DD  0                           ;隐藏扇区数（一般为0个）
DD  2880                        ;总扇区数（一般为2880个扇区）
DB  0                           ;驱动器号（一般为0）
DB  0                           ;保留字节（一般为0）
DB  0x29                        ;扩展引导标志（一般为0x29）
DD  0x12345678                  ;卷序列号（随便写）
DB  "HELLO-OS   "               ;卷标（随便写）（11字节）
DB  "FAT12   "                  ;文件系统类型（必须为"FAT12"）（8字节）
RESB    18

; 程序主体
entry:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX
    MOV ES, AX

    MOV SI, msg
putloop:
    MOV AL, [SI]
    ADD SI, 1
    CMP AL, 0
    JE  fin
    MOV AH, 0x0e
    MOV BX, 15
    INT 0x10
    JMP putloop
fin:
    HLT
    JMP fin

; 信息显示部分
msg:
    DB  0x0a, 0x0a
    DB  "Hello, world"
    DB  0x0a
    DB  0

    RESB    0x7dfe-$                ;现在这个$变成7c00+当前的地址，说明ORG会是程序从他定义的地址开始
    DB  0x55, 0xaa