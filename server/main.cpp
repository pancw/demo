#include <iostream>
#include <cstdio>
#include "engine_base.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

int main(void)
{
	printf("hello world\n");
	int port = 1248;
	engine_base::init();
	engine_base::clientListen(port);

	engine_base::loop();
	engine_base::release();
	return 0;
}
