#ifndef _LUAHTTPSRV_HPP_
#define _LUAHTTPSRV_HPP_

#include <map>
#include <string>
#include <stdlib.h>
#include <string.h>
#include <lua.hpp>

namespace httpsrv {
	extern void startHttpSrv(int);
	extern void release();
}

#endif

