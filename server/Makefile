MACHINE=$(shell uname -m)
ifeq ($(MACHINE), x86_64)
MBITS=64
else
MBITS=32
endif

ROOT=.

SRC=main.cpp \
	engine_base.cpp \
	client_mgr.cpp \
	module/dns.cpp \
	module/md5.cpp \
	module/http.cpp \
	module/http_srv.cpp \
	module/lmongoc.cpp

-include $(ROOT)/Makefile.conf

ENGINE_BIN=$(ROOT)/bin/game_server

CPPFLAGS=$(FLAG) -I $(ROOT)/include

all:.dep $(ENGINE_BIN)

-include .dep

$(ENGINE_BIN):$(OBJ) $(LIBEVENT) $(LIBLUA) $(MONGOC)
	$(CXX) $(CPPFLAGS) -o $@ $^ -lrt -ldl -lssl -lpthread -lcrypto

clean:
	find . -name "*.o" -delete
	rm $(ENGINE_BIN)
