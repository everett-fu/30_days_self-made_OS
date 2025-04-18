本文原版为中文版，英文版为ai翻译而成，如有翻译错误请联系我。
**阅读其他版本: [English](README.md), [中文](README_zh.md).**

# NubulaOS
本项目是基于川合秀实《30天自制操作系统》之中haribOS操作系统的实现、移植与优化。
</br>该项目为本人毕业设计项目。

## 项目介绍
原代码为川合秀实编写的HaribOS（纸娃娃操作系统），该作者为了教学方便而使用了一套非标准的开发工具，使得该操作系统几乎无法在任何非Windows平台上运行，
虽然作者给出了Docker的运行方式与Linux的移植的软件包，但终归不是标准的开发工具，使得这套开发流程无法迁移到现有的一些操作系统开发之中。加上HaribOS
编写时间较久，川合秀实编写这个系统的时候用的是软盘+MBR+BIOS，在当今社会基本没有人会使用软盘来启动操作系统，较为现代的启动方式为U盘/光盘+GPT+UEFI
启动。

本项目将在源代码的基础上使用NASM+GCC+Qemu来重写部分代码，并将部分算法替换成较为现代的算法。希望这次的重写能够为后来的朋友提供一些帮助，因此我将
本项目名命名为NubulaOS，意思为星云，我将拨开云雾并为后来者提供一些帮助。

本项目将先完成川合秀实的代码，然后再进行拆分、重写、优化。现已完成0~25天的内容。

## 项目特色
+ 采用GCC、NASM作为工具链，可跨平台使用
+ 采用U盘+GPT+UEFI启动
+ 优化部分算法
+ 不依赖于作者自定义HRB文件格式，支持ELF文件格式
+ 支持部分C标准库函数的简单实现（libc文件）
+ 头文件拆分，结构更为清晰
+ 中文键盘支持

## 环境依赖
+ `nasm`
+ `gcc`
+ `binutils`
+ `qemu-system-i386`
+ `mtools`

### 项目目录结构
├─`build`：中间文件<br>
├─`images`：镜像文件<br>
├─`src`：项目源代码<br>
└─`z_tools`：辅助构建工具<br>

### 运行
现阶段还是沿用川合秀实的代码，并没有做标准化工具的重写，因此现在还需要使用川合秀实的开发工具，仅限Windows平台使用，具体的使用过程如下：
1. 克隆本项目
2. 下载川合秀的z_tools的工具包，并放置在项目目录底下（与src同一层）
3. 打开src中的!cons_nt.bat，并输入`make run`打开Qume
4. 退出只需要关闭Qume

## 项目进度

+ [X] 第1天：Hello world
+ [X] 第2天：汇编与Makefile
+ [X] 第3天：进入32位模式
+ [X] 第4天：画面显示
+ [X] 第5天：GDT/IDT
+ [X] 第6天：中断处理
+ [x] 第7天：FIFO
+ [X] 第8天：鼠标与32位模式
+ [X] 第9天：内存管理
+ [X] 第10天：窗口叠加
+ [X] 第11天：窗口处理
+ [X] 第12天：定时器1
+ [X] 第13天：定时器2
+ [X] 第14天：键盘输入
+ [X] 第15天：多任务1
+ [X] 第16天：多任务2
+ [X] 第17天：命令行窗口
+ [X] 第18天：命令行命令
+ [X] 第19天：应用程序
+ [X] 第20天：API
+ [X] 第21天：保护操作系统
+ [X] 第22天：C语言应用程序（修改为ELF格式）
+ [X] 第23天：应用程序图形处理
+ [X] 第24天：窗口操作
+ [X] 第25天：更多窗口
+ [ ] 第26天：窗口操作提速
+ [ ] 第27天：LDT与库（未按书上处理）
+ [ ] 第28天：文件操作与文字显示（不包含日文显示部分）

## 待解决的问题
+ [ ] 完成川合秀实的源代码
+ [ ] 采用GCC、NASM作为工具链，可跨平台使用
+ [ ] 采用U盘+GPT+UEFI启动
+ [ ] 优化部分算法
+ [ ] 不依赖于作者自定义HRB文件格式，支持ELF文件格式
+ [ ] 支持部分C标准库函数的简单实现（libc文件）
+ [ ] 头文件拆分，结构更为清晰
+ [ ] 中文键盘支持
