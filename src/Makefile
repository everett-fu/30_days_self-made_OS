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
	$(BUILDPATH)sheet.obj $(BUILDPATH)timer.obj $(BUILDPATH)mtask.obj \
	$(BUILDPATH)window.obj $(BUILDPATH)console.obj $(BUILDPATH)file.obj \

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

$(BUILDPATH)a_nask.obj : a_nask.nas Makefile
	$(NASK) a_nask.nas $(BUILDPATH)a_nask.obj $(BUILDPATH)a_nask.lst

$(BUILDPATH)a.bim : $(BUILDPATH)a.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)a.bim map:$(BUILDPATH)a.map $(BUILDPATH)a.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)a.hrb : $(BUILDPATH)a.bim Makefile
	$(BIM2HRB) $(BUILDPATH)a.bim $(BUILDPATH)a.hrb 0

$(BUILDPATH)bug1.bim : $(BUILDPATH)bug1.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)bug1.bim map:$(BUILDPATH)bug1.map $(BUILDPATH)bug1.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)bug1.hrb : $(BUILDPATH)bug1.bim Makefile
	$(BIM2HRB) $(BUILDPATH)bug1.bim $(BUILDPATH)bug1.hrb 0

$(BUILDPATH)bug2.bim : $(BUILDPATH)bug2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)bug2.bim map:$(BUILDPATH)bug2.map $(BUILDPATH)bug2.obj

$(BUILDPATH)bug2.hrb : $(BUILDPATH)bug2.bim Makefile
	$(BIM2HRB) $(BUILDPATH)bug2.bim $(BUILDPATH)bug2.hrb 0

$(BUILDPATH)%.obj : %.nas Makefile
	$(NASK) $< $@ $(BUILDPATH)$*.lst

$(BUILDPATH)hello.bim : $(BUILDPATH)hello.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)hello.bim map:$(BUILDPATH)hello.map $(BUILDPATH)hello.obj

$(BUILDPATH)hello.hrb : $(BUILDPATH)hello.bim Makefile
	$(BIM2HRB) $(BUILDPATH)hello.bim $(BUILDPATH)hello.hrb 0

$(BUILDPATH)hello2.bim : $(BUILDPATH)hello2.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)hello2.bim map:$(BUILDPATH)hello2.map $(BUILDPATH)hello2.obj

$(BUILDPATH)hello2.hrb : $(BUILDPATH)hello2.bim Makefile
	$(BIM2HRB) $(BUILDPATH)hello2.bim $(BUILDPATH)hello2.hrb 0

$(BUILDPATH)hello3.bim : $(BUILDPATH)hello3.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)hello3.bim map:$(BUILDPATH)hello3.map $(BUILDPATH)hello3.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)hello3.hrb : $(BUILDPATH)hello3.bim Makefile
	$(BIM2HRB) $(BUILDPATH)hello3.bim $(BUILDPATH)hello3.hrb 0

$(BUILDPATH)hello4.bim : $(BUILDPATH)hello4.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)hello4.bim map:$(BUILDPATH)hello4.map $(BUILDPATH)hello4.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)hello4.hrb : $(BUILDPATH)hello4.bim Makefile
	$(BIM2HRB) $(BUILDPATH)hello4.bim $(BUILDPATH)hello4.hrb 0

$(BUILDPATH)hello5.bim : $(BUILDPATH)hello5.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)hello5.bim stack:1k map:$(BUILDPATH)hello5.map $(BUILDPATH)hello5.obj

$(BUILDPATH)hello5.hrb : $(BUILDPATH)hello5.bim Makefile
	$(BIM2HRB) $(BUILDPATH)hello5.bim $(BUILDPATH)hello5.hrb 0

$(BUILDPATH)winhelo.bim : $(BUILDPATH)winhelo.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)winhelo.bim stack:1k map:$(BUILDPATH)winhelo.map $(BUILDPATH)winhelo.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)winhelo.hrb : $(BUILDPATH)winhelo.bim Makefile
	$(BIM2HRB) $(BUILDPATH)winhelo.bim $(BUILDPATH)winhelo.hrb 0

$(BUILDPATH)winhelo2.bim : $(BUILDPATH)winhelo2.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)winhelo2.bim stack:1k map:$(BUILDPATH)winhelo2.map $(BUILDPATH)winhelo2.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)winhelo2.hrb : $(BUILDPATH)winhelo2.bim Makefile
	$(BIM2HRB) $(BUILDPATH)winhelo2.bim $(BUILDPATH)winhelo2.hrb 0

$(BUILDPATH)winhelo3.bim : $(BUILDPATH)winhelo3.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)winhelo3.bim stack:1k map:$(BUILDPATH)winhelo3.map $(BUILDPATH)winhelo3.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)winhelo3.hrb : $(BUILDPATH)winhelo3.bim Makefile
	$(BIM2HRB) $(BUILDPATH)winhelo3.bim $(BUILDPATH)winhelo3.hrb 40k

$(BUILDPATH)star1.bim : $(BUILDPATH)star1.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)star1.bim stack:1k map:$(BUILDPATH)star1.map $(BUILDPATH)star1.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)star1.hrb : $(BUILDPATH)star1.bim Makefile
	$(BIM2HRB) $(BUILDPATH)star1.bim $(BUILDPATH)star1.hrb 100k

$(BUILDPATH)stars.bim : $(BUILDPATH)stars.obj $(BUILDPATH)a_nask.obj Makefile
	$(OBJ2BIM) @$(RULEFILE) out:$(BUILDPATH)stars.bim stack:1k map:$(BUILDPATH)stars.map $(BUILDPATH)stars.obj $(BUILDPATH)a_nask.obj

$(BUILDPATH)stars.hrb : $(BUILDPATH)stars.bim Makefile
	$(BIM2HRB) $(BUILDPATH)stars.bim $(BUILDPATH)stars.hrb 100k

$(IMAGESPATH)haribote.img : $(BUILDPATH)ipl10.bin $(BUILDPATH)haribote.sys $(BUILDPATH)hello.hrb $(BUILDPATH)hello2.hrb \
							$(BUILDPATH)a.hrb $(BUILDPATH)bug1.hrb $(BUILDPATH)bug2.hrb $(BUILDPATH)hello3.hrb \
							$(BUILDPATH)hello4.hrb $(BUILDPATH)hello5.hrb $(BUILDPATH)winhelo.hrb $(BUILDPATH)winhelo2.hrb \
							$(BUILDPATH)winhelo3.hrb $(BUILDPATH)star1.hrb $(BUILDPATH)stars.hrb \
							Makefile
	$(EDIMG)   imgin:../z_tools/fdimg0at.tek \
		wbinimg src:$(BUILDPATH)ipl10.bin len:512 from:0 to:0 \
		copy from:$(BUILDPATH)haribote.sys to:@: \
		copy from:ipl10.nas to:@: \
		copy from:make.bat to:@: \
		copy from:$(BUILDPATH)hello.hrb to:@: \
		copy from:$(BUILDPATH)hello2.hrb to:@: \
		copy from:$(BUILDPATH)a.hrb to:@: \
		copy from:$(BUILDPATH)bug1.hrb to:@: \
		copy from:$(BUILDPATH)bug2.hrb to:@: \
		copy from:$(BUILDPATH)hello3.hrb to:@: \
		copy from:$(BUILDPATH)hello4.hrb to:@: \
		copy from:$(BUILDPATH)hello5.hrb to:@: \
		copy from:$(BUILDPATH)winhelo.hrb to:@: \
		copy from:$(BUILDPATH)winhelo2.hrb to:@: \
		copy from:$(BUILDPATH)winhelo3.hrb to:@: \
		copy from:$(BUILDPATH)star1.hrb to:@: \
		copy from:$(BUILDPATH)stars.hrb to:@: \
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
	-$(DEL) $(BUILDPATH_W)*.hrb
	-$(DEL) $(BUILDPATH_W)*.map
	-$(DEL) $(BUILDPATH_W)*.bim
	-$(DEL) $(BUILDPATH_W)haribote.sys
	-$(DEL) ..\images\haribote.img

src_only :
	$(MAKE) clean
	-$(DEL) haribote.img