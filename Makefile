#/************************************************************************************
#***
#***	Copyright 2010-2020 Dell(18588220928@163.com), All Rights Reserved.
#***
#***	File Author: Dell, Sun Feb  5 20:51:04 CST 2010
#***
#************************************************************************************/
#

LIBNAME := libnimage
PKGCONFNAME=nimage.pc

# Installation related variables and target
PREFIX?=/usr/local
INCLUDE_PATH?=include/nimage
LIBRARY_PATH?=lib
PKGCONF_PATH?=pkgconfig
INSTALL_INCLUDE_PATH= $(PREFIX)/$(INCLUDE_PATH)
INSTALL_LIBRARY_PATH= $(PREFIX)/$(LIBRARY_PATH)
INSTALL_PKGCONF_PATH= $(INSTALL_LIBRARY_PATH)/$(PKGCONF_PATH)


INCS	:= \
	-Iinclude \
	-I/usr/local/include \

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
	source/license.c

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
all: premake staticlib

staticlib:$(OBJECTS)
	$(AR) $(ARFLAGS) $(LIBNAME).a $(OBJECTS)


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
	sudo mkdir -p ${INSTALL_INCLUDE_PATH}
	sudo cp include/*.h ${INSTALL_INCLUDE_PATH}
	sudo cp ${LIBNAME}.a ${INSTALL_LIBRARY_PATH}

	sudo mkdir -p ${INSTALL_PKGCONF_PATH}
	sudo cp ${PKGCONFNAME} ${INSTALL_PKGCONF_PATH}

premake:
	echo sudo apt-get install -y libjpeg-dev libpng-dev

