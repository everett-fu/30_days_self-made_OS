# 文件生成规则
TOOLPATH	= ../z_tools/
INCPATH		= ../z_tools/haribote/

# 中间文件目录
BUILD	= build
BUILDPATH	= ../$(BUILD)/
# 使用Windows特有的命令的时候要使用
BUILDPATH_W	= ..\$(BUILD)\\
# 镜像文件目录
IMAGES	= images
IMAGESPATH		=../$(IMAGES)/

OBJS_BOOTPACK = $(BUILDPATH)bootpack.obj $(BUILDPATH)naskfunc.obj $(BUILDPATH)hankaku.obj \
	$(BUILDPATH)graphic.obj $(BUILDPATH)dsctbl.obj $(BUILDPATH)int.obj $(BUILDPATH)fifo.obj \
	$(BUILDPATH)keyboard.obj $(BUILDPATH)mouse.obj $(BUILDPATH)memory.obj \
	$(BUILDPATH)sheet.obj $(BUILDPATH)timer.obj $(BUILDPATH)mtask.obj

# 编译工具目录
MAKE		= $(TOOLPATH)make.exe -r
NASK		= $(TOOLPATH)nask.exe
CC1			= $(TOOLPATH)cc1.exe -I$(INCPATH) -Os -Wall -quiet
GAS2NASK	= $(TOOLPATH)gas2nask.exe -a
OBJ2BIM		= $(TOOLPATH)obj2bim.exe
MAKEFONT	= $(TOOLPATH)makefont.exe
BIN2OBJ		= $(TOOLPATH)bin2obj.exe
BIM2HRB		= $(TOOLPATH)bim2hrb.exe
RULEFILE	= $(TOOLPATH)haribote/haribote.rul
EDIMG		= $(TOOLPATH)edimg.exe
IMGTOL		= $(TOOLPATH)imgtol.com
COPY		= copy
DEL			= del

default :
	$(MAKE) run

# 文件生成规则
$(BUILDPATH)ipl10.bin : ipl10.nas Makefile
	$(NASK) ipl10.nas $(BUILDPATH)ipl10.bin $(BUILDPATH)ipl10.lst

$(BUILDPATH)asmhead.bin : asmhead.nas Makefile
	$(NASK) asmhead.nas $(BUILDPATH)asmhead.bin $(BUILDPATH)asmhead.lst

$(BUILDPATH)naskfunc.obj : naskfunc.nas Makefile
	$(NASK) $< $@ $(BUILDPATH)$*.lst

$(BUILDPATH)hankaku.bin : hankaku.txt Makefile
	$(MAKEFONT) hankaku.txt $(BUILDPATH)hankaku.bin

$(BUILDPATH)hankaku.obj : $(BUILDPATH)hankaku.bin Makefile
	$(BIN2OBJ) $(BUILDPATH)hankaku.bin $(BUILDPATH)hankaku.obj _hankaku

$(BUILDPATH)bootpack.bim : $(OBJS_BOOTPACK) Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)bootpack.bim stack:3136k map:$(BUILDPATH)bootpack.map \
		$(OBJS_BOOTPACK)

$(BUILDPATH)bootpack.hrb : $(BUILDPATH)bootpack.bim Makefile
	$(BIM2HRB) $(BUILDPATH)bootpack.bim $(BUILDPATH)bootpack.hrb 0

$(BUILDPATH)haribote.sys : $(BUILDPATH)asmhead.bin $(BUILDPATH)bootpack.hrb Makefile
	copy /B $(BUILDPATH_W)asmhead.bin+$(BUILDPATH_W)bootpack.hrb $(BUILDPATH_W)haribote.sys

$(IMAGESPATH)haribote.img : $(BUILDPATH)ipl10.bin $(BUILDPATH)haribote.sys Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:$(BUILDPATH)ipl10.bin len:512 from:0 to:0 \
		copy from:$(BUILDPATH)haribote.sys to:@: \
		imgout:$(IMAGESPATH)haribote.img

# C -> GAS -> NAS -> OBJ
$(BUILDPATH)%.gas : %.c bootpack.h Makefile
	$(CC1) -o $@ $<

$(BUILDPATH)%.nas : $(BUILDPATH)%.gas Makefile
	$(GAS2NASK) $< $@

$(BUILDPATH)%.obj : $(BUILDPATH)%.nas Makefile
	$(NASK) $< $@ $(BUILDPATH)$*.lst

img :
	$(MAKE) $(IMAGESPATH)haribote.img

run :
	$(MAKE) img
	$(COPY) ..\images\haribote.img ..\z_tools\qemu\fdimage0.bin
	$(MAKE) -C ../z_tools/qemu

install :
	$(MAKE) img
	$(IMGTOL) w a: $(IMAGESPATH)haribote.img

clean :
	-$(DEL) $(BUILDPATH_W)*.bin
	-$(DEL) $(BUILDPATH_W)*.lst
	-$(DEL) $(BUILDPATH_W)*.obj
	-$(DEL) $(BUILDPATH_W)bootpack.map
	-$(DEL) $(BUILDPATH_W)bootpack.bim
	-$(DEL) $(BUILDPATH_W)bootpack.hrb
	-$(DEL) $(BUILDPATH_W)haribote.sys
	-$(DEL) ..\images\haribote.img

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img