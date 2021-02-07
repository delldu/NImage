#/************************************************************************************
#***
#***	Copyright 2020 Dell(18588220928g@163.com), All Rights Reserved.
#***
#***	File Author: Dell, 2020-11-22 02:26:07
#***
#************************************************************************************/
#
INSTALL_DIR := /usr/local

TARGET := nimage

SOURCE :=  \
	main.c \

INCS	:= -I../include

LIBS	:= \
	-Wl,-Bstatic \
	-L.. -lnimage \
	-lnanomsg -lanl \
	-lmsgpackc \
	-ljpeg -lpng \
	-lz \
	-Wl,-Bdynamic \
	-lpthread -lm


# LIBS	:= \
# 	-L.. -lnimage \
# 	-lnanomsg -lanl \
# 	-lmsgpackc \
# 	-ljpeg -lpng \
# 	-lpthread -lm

DEFINES :=

CFLAGS := -O2 -fPIC -Wall -Wextra
LDFLAGS := -fPIC

#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************
CFLAGS   := ${CFLAGS} ${DEFINES}
CXXFLAGS := ${CXXFLAGS} ${DEFINES}

OBJECT := $(addsuffix .o,$(basename ${SOURCE}))

#****************************************************************************
# Compile block
#****************************************************************************
all: ${TARGET}

${TARGET}: ${OBJECT}
	${CC} ${LDFLAGS} -o ${TARGET} ${OBJECT} ${LIBS}

install:
# 	${STRIP} ${TARGET}
	sudo install -m 755 ${TARGET} ${INSTALL_DIR}/bin
	sudo cp nimage.service /lib/systemd/system/
	sudo systemctl daemon-reload


#****************************************************************************
# common rules
#****************************************************************************
%.o : %.cpp
	${CXX} ${CXXFLAGS} ${INCS} -c $< -o $@

%.o : %.c
	${CC} ${CFLAGS} ${INCS} -c $< -o $@

#****************************************************************************
# Depend block
#****************************************************************************
depend:

clean:
	rm -f core ${OBJECT} ${TARGET}
