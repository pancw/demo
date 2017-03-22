#ifndef _LUAHTTP_HPP_
#define _LUAHTTP_HPP_

#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <lua.hpp>

namespace pack_http {
	void luaopen_httplib(lua_State* L);
}

#endif

