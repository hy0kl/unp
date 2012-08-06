# VPATH = src:../headers
# %: 匹配零或若干字符
# $< 表示所有的依赖目标集
# $@ 表示目标集
# 用 @ 字符在命令行前,此命令将不被 make 显示出来
# 在 Makefile 的命令行前加一个减号 - (在 Tab 键之后),标记为不管命令出不出错都认为是成功的

LIBDIR = -levent

GCC = gcc
GCCFLAGS = -g -finline-functions -Wall -Winline -pipe

TARGET = sug-server
TOOL   = hash
CONTRIB= hz2py

OBJS1  = event-http.o util.o
OBJS2  = hash.o util.o

all : $(TARGET) $(TOOL)
	@echo "Start compile all"
	rm -f *.o

$(TARGET) : $(OBJS1)
	@echo "start compile server"
	$(GCC) -g -o $@ $^ $(LIBDIR)

$(TOOL) : $(OBJS2)
	@echo "start compile tool"
	$(GCC) -g -o $@ $^ $(LIBDIR)

%.o : %.c
	$(GCC) $(GCCFLAGS) -c $< -o $@

.PHONY: clean
clean:
	rm -rf $(TARGET) $(OBJS1) $(OBJS2) $(TOOL) $(CONTRIB)

.PHONY: cleanobj
cleanobj:
	rm -rf $(OBJS1) $(OBJS2)
