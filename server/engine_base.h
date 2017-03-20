
#ifndef __EVENTBASE_H__
#define __EVENTBASE_H__

#include <sys/socket.h>

#include "event2/event.h"
#include "event2/listener.h"
#include "event2/bufferevent.h"
#include "event2/thread.h"
#include "event2/event-config.h"

#include <signal.h>
#include <cstdio>
#include <event.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <iostream>
#include <string.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace engine_base {

	extern struct event_base *base;
	extern void init();
	extern bool clientListen(int port);
	extern void release();
	extern void loop();
}

#endif
