#/************************************************************************************
#***
#***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
#***
#***	File Author: Dell, 2021-02-07 13:37:06
#***
#************************************************************************************/
#

INSTALL_DIR := /usr/local

LIB_NAME := libmsgpackc

INCS	:= -Iinclude
	# -I$(BUILD_DIR)/include
SOURCE :=  \
	src/unpack.c \
	src/version.c \
	src/zone.c \
	src/objectc.c \
	src/vrefbuffer.c 

DEFINES := 
CFLAGS := -O2 -fPIC -Wall -Wextra
LDFLAGS := -fPIC

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
	mkdir -p ${INSTALL_DIR}/include/msgpack
	cp include/* ${INSTALL_DIR}/include -R
	cp ${LIB_NAME}.a ${INSTALL_DIR}/lib

