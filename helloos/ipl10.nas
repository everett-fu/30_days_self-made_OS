; haribote-ipl
; TAB = 4

CYLS    EQU 10

ORG 0x7c00

; 以下这段是标准FAT12格式软盘专用的引导扇区代码
JMP entry
DB  0x90

DB  "HARIBOTE"                  ;启动区的名称可以是任意的字符串（8字符）
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
DB  "HARIBOTEOS "               ;卷标（随便写）（11字节）
DB  "FAT12   "                  ;文件系统类型（必须为"FAT12"）（8字节）
RESB    18

; 程序主体
entry:
    MOV AX, 0
    MOV SS, AX
    MOV SP, 0x7c00
    MOV DS, AX

;读磁盘
    MOV AX, 0x0820
    MOV ES, AX
    ;MOV BX, 0
    MOV DH, 0                   ;磁头0
    MOV CH, 0                   ;柱面0
    MOV CL, 2                   ;扇区2
readloop:
    MOV SI, 0                   ;记录失败的次数
retry:
    MOV AH, 0x02                ;读盘
    MOV AL, 1                   ;操作1个扇区
    MOV BX, 0
    MOV DL, 0x00                ;驱动器号
    INT 0x13                    ;如果有错误将CF=1，AH设置为0，AL为错误码，没有错误CF=0，CF为进位标志
    JNC next                     ;如果没有错误，跳转到fin
    ;有错误，计数加一，如果失败次数超过5次，跳转到错误处理
    ADD SI, 1
    CMP SI, 5
    JAE error                   ;如果失败次数>=5次，跳转到错误处理
    ;复位磁盘
    MOV AH, 0x00
    MOV DL, 0x00
    INT 0x13
    JMP retry

next:
    ;ADD BX, 0x200
    MOV AX, ES
    ADD AX, 0x0020
    MOV ES, AX
    ADD CL, 1
    CMP CL, 18
    JBE readloop
    MOV CL, 1
    ADD DH, 1
    CMP DH, 2
    JB  readloop
    MOV DH, 0
    ADD CH, 1
    CMP CH, CYLS
    JB  readloop

    MOV [0x0ff0], CH
    JMP 0xc200

error:
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
    DB  "load error"
    DB  0x0a
    DB  0

    RESB    0x7dfe-$                ;现在这个$变成7c00+当前的地址，说明ORG会是程序从他定义的地址开始
    DB  0x55, 0xaa