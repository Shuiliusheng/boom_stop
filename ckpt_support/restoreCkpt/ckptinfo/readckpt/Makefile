

CC = riscv64-unknown-linux-gnu-gcc 
OBJDUMP = riscv64-unknown-linux-gnu-objdump
CFLAGS = -static -O2 #-DSHOWLOG 
DIR_SRC = .
DIR_INC = .
TRAGET = readckpt.riscv

SRC = $(wildcard ${DIR_SRC}/*.cpp)
OBJ = $(patsubst %.cpp, %.o, $(notdir ${SRC}))
ELSE = $(patsubst takeover_syscall.o, , ${OBJ})

all: ${TRAGET}

${TRAGET}: takeover_syscall.o ${ELSE}
	@echo ${ELSE}
	${CC} ${CFLAGS} -o $@  $^

%.o: ${DIR_SRC}/%.cpp
	${CC} ${CFLAGS} -o $@ -c $<


clean:
	rm -rf ${OBJ}

asm:
	${OBJDUMP} -d readinfo.riscv >read.s
