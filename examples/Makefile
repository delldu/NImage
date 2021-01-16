#/************************************************************************************
#***
#***	Copyright 2010-2020 Dell(18588220928@163.com), All Rights Reserved.
#***
#***	File Author: Dell, Sun Feb  5 20:51:04 CST 2010
#***
#************************************************************************************/
#

# Configurations
APPLETS := $(shell ./makebox --source)

TARGET := imagebox
INSTALL_DIR := /usr/local

SOURCE := main.c
SOURCE += $(APPLETS)

INCS	:= -I../include \
	$(shell gimptool-2.0 --cflags)

LIBS	:= -L.. -lnimage -ljpeg -lpng -lm  

CFLAGS := -O2 -fPIC
LDFLAGS := -fPIC

# export LD_LIBRARY_PATH=${LD_LIBRARY_PATH}:..


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************
CFLAGS   := ${CFLAGS} ${DEFINES}
CXXFLAGS := ${CXXFLAGS} ${DEFINES}

OBJECT := $(addsuffix .o,$(basename ${SOURCE}))

all: ${TARGET} 
# install

#****************************************************************************
# Compile block
#****************************************************************************
${TARGET}: applets.h ${OBJECT}
	${CC} -o ${TARGET} ${LDFLAGS} ${OBJECT} ${LIBS}

applets.h: imagebox.config
	./makebox --table > applets.h

install:
	#${STRIP} ${TARGET}
	install -m 755 ${TARGET} ${INSTALL_DIR}/bin
	./makebox --links ${INSTALL_DIR}/bin

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
	rm -f core ${OBJECT}
	rm -f core ${TARGET}
	rm -f applets.h

