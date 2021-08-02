#/************************************************************************************
#***
#***	Copyright 2010-2020 Dell(18588220928@163.com), All Rights Reserved.
#***
#***	File Author: Dell, Sun Feb  5 20:51:04 CST 2010
#***
#************************************************************************************/
#

LIB_NAME := libnimage
INSTALL_DIR := /usr/local

INCS	:= \
	-Iinclude \
	-Imsgpackc/include \
	-I/usr/local/include

SOURCE :=  \
	source/color.c \
	source/frame.c \
	source/hough.c \
	source/matrix.c \
	source/shape.c \
	source/text.c \
	source/vector.c \
	source/common.c  \
	source/image.c \
	source/motion.c \
	source/texture.c \
	source/video.c \
	source/blend.c \
	source/filter.c \
	source/hash64.c \
	source/retinex.c \
	source/histogram.c \
	source/mask.c \
	source/tensor.c \
	source/nnmsg.c

DEFINES := 
CFLAGS := -O2 -fPIC -Wall -Wextra
LDFLAGS := -fPIC -ljpeg -lpng
 


#****************************************************************************
# Makefile code common to all platforms
#****************************************************************************
CFLAGS   := ${CFLAGS} ${DEFINES}
CXXFLAGS := ${CXXFLAGS} ${DEFINES}
OBJECTS := $(addsuffix .o,$(basename ${SOURCE}))

#****************************************************************************
# Compile block
#****************************************************************************
all: staticlib

sharelib: $(OBJECTS)
	$(LD) $(LDFLAGS) -shared -soname $(LIB_NAME).so -o $(LIB_NAME).so $(OBJECTS)

staticlib:$(OBJECTS)
	$(AR) $(ARFLAGS) $(LIB_NAME).a $(OBJECTS)


#****************************************************************************
# Depend block
#****************************************************************************
depend:

#****************************************************************************
# common rules
#****************************************************************************
%.o : %.cpp
	${CXX} ${CXXFLAGS} ${INCS} -c $< -o $@

%.o : %.c
	${CC} ${CFLAGS} ${INCS} -c $< -o $@


clean:
	rm -rf *.a *.so *.o $(OBJECTS)

install:
	make -C msgpackc && make -C msgpackc install
	make -C nanomsg && make -C nanomsg install

	mkdir -p ${INSTALL_DIR}/include/nimage
	cp include/*.h ${INSTALL_DIR}/include/nimage 
	cp ${LIB_NAME}.a ${INSTALL_DIR}/lib
	cp nimagetool ${INSTALL_DIR}/bin



