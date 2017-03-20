#include <iostream>
#include <cstdio>
#include "engine_base.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

lua_State *GlobalL;

int error_fun(lua_State *state)
{
	std::string result;
	const char *tmp = lua_tostring(state, -1); // error_msg
	if (tmp) {
		result = tmp;
	}

	lua_getglobal(state, "debug"); // error_msg, debug
	lua_getfield(state, -1, "traceback"); // error_msg, debug, traceback
	lua_call(state, 0, 1); // error_msg, traceback

	tmp = lua_tostring(state, -1);
	if (tmp) {
		result = result + "\n" + tmp;
	}

	lua_pushstring(state, result.c_str()); // push result
	return 1;
}

static void InitLuaLib()
{
	GlobalL = luaL_newstate();
	lua_gc(GlobalL, LUA_GCSTOP, 0);	
	luaL_openlibs(GlobalL);
	lua_gc(GlobalL, LUA_GCRESTART, 0);
}

static int ReadConfig(int &port)
{
	lua_getglobal(GlobalL, "CONFIG_TBL");	
	if (!lua_istable(GlobalL, -1))
	{
		lua_pop(GlobalL, 1);
		return 1;
	}

	lua_pushstring(GlobalL, "port");
	lua_rawget(GlobalL, -2);
	port = lua_tonumber(GlobalL, -1);	
	lua_pop(GlobalL, 1);

	lua_pop(GlobalL, 1);
	return 0;
}

int main(void)
{
	printf("hello world\n");

	if (chdir("../logic") == -1)
	{
		fprintf(stderr, "bad logic path to dir:%s\n", "../logic");
		return 1;
	}

	engine_base::init();
	InitLuaLib();

	int err = luaL_loadfile(GlobalL, "main.lua");	
	if (err)
	{
		fprintf(stderr, "%s\n", luaL_checkstring(GlobalL, -1));
		return 1;
	}

	int ret = lua_pcall(GlobalL, 0, 0, 0);
	if (ret)
	{
		fprintf(stderr, "call main error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}

	int port = 0;
	ret = ReadConfig(port);
	if (ret)
	{
		fprintf(stderr, "read config error:%s\n", lua_tostring(GlobalL, -1));
		return 1;
	}


	if (engine_base::clientListen(port))
	{
		printf("Accepting client connections on port %d\n", port);	
	}
	else
	{
		fprintf(stderr, "clientListen error.\n");
		return 1;
	}

	engine_base::loop();
	engine_base::release();
	return 0;
}
