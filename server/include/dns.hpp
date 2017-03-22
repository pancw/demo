
#ifndef __DNS_H_
#define __DNS_H_

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

namespace pack_dns {
	int init_dns();
	void luaopen_dnslib(lua_State* L);
}

#endif//__DNS_H_
