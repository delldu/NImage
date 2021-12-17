#/************************************************************************************
#***
#***	Copyright 2021 Dell(18588220928g@163.com), All Rights Reserved.
#***
#***	File Author: Dell, 2021-05-17 11:43:32
#***
#************************************************************************************/
#
INSTALL_DIR := /usr/local

LIB_NAME := libnanomsg

INCS	:= 
	# -I$(BUILD_DIR)/include
SOURCE :=  \
	src/aio/ctx.c \
	src/aio/fsm.c \
	src/aio/poller.c \
	src/aio/pool.c \
	src/aio/timer.c \
	src/aio/timerset.c \
	src/aio/usock.c \
	src/aio/worker.c \
	src/core/ep.c \
	src/core/global.c \
	src/core/pipe.c \
	src/core/poll.c \
	src/core/sockbase.c \
	src/core/sock.c \
	src/core/symbol.c \
	src/devices/device.c \
	src/protocols/bus/bus.c \
	src/protocols/bus/xbus.c \
	src/protocols/pair/pair.c \
	src/protocols/pair/xpair.c \
	src/protocols/pipeline/pull.c \
	src/protocols/pipeline/push.c \
	src/protocols/pipeline/xpull.c \
	src/protocols/pipeline/xpush.c \
	src/protocols/pubsub/pub.c \
	src/protocols/pubsub/sub.c \
	src/protocols/pubsub/trie.c \
	src/protocols/pubsub/xpub.c \
	src/protocols/pubsub/xsub.c \
	src/protocols/reqrep/rep.c \
	src/protocols/reqrep/req.c \
	src/protocols/reqrep/task.c \
	src/protocols/reqrep/xrep.c \
	src/protocols/reqrep/xreq.c \
	src/protocols/survey/respondent.c \
	src/protocols/survey/surveyor.c \
	src/protocols/survey/xrespondent.c \
	src/protocols/survey/xsurveyor.c \
	src/protocols/utils/dist.c \
	src/protocols/utils/excl.c \
	src/protocols/utils/fq.c \
	src/protocols/utils/lb.c \
	src/protocols/utils/priolist.c \
	src/transports/inproc/binproc.c \
	src/transports/inproc/cinproc.c \
	src/transports/inproc/inproc.c \
	src/transports/inproc/ins.c \
	src/transports/inproc/msgqueue.c \
	src/transports/inproc/sinproc.c \
	src/transports/ipc/aipc.c \
	src/transports/ipc/bipc.c \
	src/transports/ipc/cipc.c \
	src/transports/ipc/ipc.c \
	src/transports/ipc/sipc.c \
	src/transports/tcp/atcp.c \
	src/transports/tcp/btcp.c \
	src/transports/tcp/ctcp.c \
	src/transports/tcp/stcp.c \
	src/transports/tcp/tcp.c \
	src/transports/utils/backoff.c \
	src/transports/utils/base64.c \
	src/transports/utils/dns.c \
	src/transports/utils/iface.c \
	src/transports/utils/literal.c \
	src/transports/utils/port.c \
	src/transports/utils/streamhdr.c \
	src/transports/ws/aws.c \
	src/transports/ws/bws.c \
	src/transports/ws/cws.c \
	src/transports/ws/sha1.c \
	src/transports/ws/sws.c \
	src/transports/ws/ws.c \
	src/transports/ws/ws_handshake.c \
	src/utils/alloc.c \
	src/utils/atomic.c \
	src/utils/chunk.c \
	src/utils/chunkref.c \
	src/utils/clock.c \
	src/utils/closefd.c \
	src/utils/condvar.c \
	src/utils/efd.c \
	src/utils/err.c \
	src/utils/hash.c \
	src/utils/list.c \
	src/utils/msg.c \
	src/utils/mutex.c \
	src/utils/once.c \
	src/utils/queue.c \
	src/utils/random.c \
	src/utils/sem.c \
	src/utils/sleep.c \
	src/utils/strcasecmp.c \
	src/utils/strcasestr.c \
	src/utils/strncasecmp.c \
	src/utils/thread.c \
	src/utils/wire.c \

DEFINES := \
	-DNN_HAVE_ACCEPT4=1 \
	-DNN_HAVE_BACKTRACE=1 \
	-DNN_HAVE_CLOCK_GETTIME=1 \
	-DNN_HAVE_CLOCK_MONOTONIC=1 \
	-DNN_HAVE_EPOLL=1 \
	-DNN_HAVE_EVENTFD=1 \
	-DNN_HAVE_GCC_ATOMIC_BUILTINS \
	-DNN_HAVE_GETADDRINFO_A=1 \
	-DNN_HAVE_LIBNSL=1 \
	-DNN_HAVE_LINUX \
	-DNN_HAVE_MSG_CONTROL=1 \
	-DNN_HAVE_PIPE2=1 \
	-DNN_HAVE_PIPE=1 \
	-DNN_HAVE_POLL=1 \
	-DNN_HAVE_SEMAPHORE \
	-DNN_HAVE_SEMAPHORE_PTHREAD=1 \
	-DNN_HAVE_SOCKETPAIR=1 \
	-DNN_HAVE_UNIX_SOCKETS=1 \
	-DNN_MAX_SOCKETS=512 \
	-DNN_SHARED_LIB \
	-DNN_USE_EPOLL \
	-DNN_USE_EVENTFD \
	-D_GNU_SOURCE \
	-D_POSIX_PTHREAD_SEMANTICS \
	-D_REENTRANT \
	-D_THREAD_SAFE \
	-Dnanomsg_EXPORTS

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
	sudo mkdir -p ${INSTALL_DIR}/include/nanomsg
	sudo cp src/*.h ${INSTALL_DIR}/include/nanomsg 
	sudo cp ${LIB_NAME}.a ${INSTALL_DIR}/lib
	
