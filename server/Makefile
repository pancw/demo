MACHINE=$(shell uname -m)
ifeq ($(MACHINE), x86_64)
MBITS=64
else
MBITS=32
endif

ROOT=.
EXTLIB=$(ROOT)/../extlib/install

LIBEVENT=$(EXTLIB)/libevent/lib/libevent.a $(EXTLIB)/libevent/lib/libevent_core.a \
	$(EXTLIB)/libevent/lib/libevent_extra.a $(EXTLIB)/libevent/lib/libevent_pthreads.a

LIBLUA=$(EXTLIB)/lua/lib/liblua.a

FLAG= -g -std=c++11 -m$(MBITS) -Wall -o0
CPPFLAGS=$(FLAG) -I $(EXTLIB)/lua/include -I $(EXTLIB)/libevent/include -I include

SRC=main.cpp engine_base.cpp client_mgr.cpp
OBJ=$(subst .cpp,.o,$(SRC))

ENGINE_BIN=$(ROOT)/bin/game_server

all:$(ENGINE_BIN)

$(ENGINE_BIN):$(OBJ) $(LIBEVENT) $(LIBLUA)
	$(CXX) $(CPPFLAGS) -o $@ $^ -lssl -lrt -ldl

clean:
	find . -name "*.o" -delete
	rm $(ENGINE_BIN)
