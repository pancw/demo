#ifndef _LUAMD5_HPP_
#define _LUAMD5_HPP_

#include <lua.hpp>

namespace pack_md5 
{
	void luaopen_md5lib(lua_State* L);

	typedef unsigned char md5_byte_t; /* 8-bit byte */
	typedef unsigned int md5_word_t; /* 32-bit word */

	/* Define the state of the MD5 Algorithm. */
	typedef struct md5_state_s {
	    md5_word_t count[2];	/* message length in bits, lsw first */
	    md5_word_t abcd[4];		/* digest buffer */
	    md5_byte_t buf[64];		/* accumulate block */
	} md5_state_t;
}

#endif /* md5_INCLUDED */

