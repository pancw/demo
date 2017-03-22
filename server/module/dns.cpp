#include <netinet/in.h>
#include <arpa/inet.h>
#include <evdns.h>
#include "dns.hpp"


extern lua_State *GlobalL;

namespace pack_dns {
	typedef union resolv_arg {
		unsigned int i;
		void * p;
	}resolv_arg_t;

	void callback(int result, char type, int count, int ttl, void *addresses, void *_arg)
	{
		int top = lua_gettop(GlobalL);
		resolv_arg_t arg;
		arg.p = _arg;
		unsigned int cbid = arg.i;
		lua_getglobal(GlobalL, "__G__TRACKBACK__");
		lua_getglobal(GlobalL, "HTTP");
		lua_pushstring(GlobalL, "onDnsCB");
		lua_rawget(GlobalL, -2);

		if (!lua_isfunction(GlobalL, -1)) {
			printf("dns lua callback function is not valid:%d", cbid);
			lua_settop(GlobalL, top);
			return;
		}

		lua_pushnumber(GlobalL, cbid);
		
		if (result==DNS_ERR_NONE && type==EVDNS_TYPE_A && count!=0) {
			const char * ip = inet_ntoa(((struct in_addr *)addresses)[0]);
			lua_pushstring(GlobalL, ip);
		} else {
			lua_pushnil(GlobalL);
		}
		if (lua_pcall(GlobalL, 2, 0, -5) != 0)
		{
			printf("dns callback error\n");
		}
		lua_settop(GlobalL, top);
	}

	int init_dns()
	{
		if (evdns_init()) 
		{
			printf("evdns init failed");	
			return -2;
		}

		return 0;
	}

	static int resolve(lua_State * L)
	{
		const char * name = luaL_checkstring(L, 1);

		resolv_arg_t arg;
		arg.i = lua_tonumber(L, 2);

		evdns_resolve_ipv4(name, 0, callback, arg.p);
		return 0;
	}

	const luaL_reg dns[] = 
	{
		{"resolve",  resolve},
		{NULL, NULL},
	};

	void luaopen_dnslib(lua_State* L)
	{
		luaL_register(L, "ldns", dns);
	}
}

