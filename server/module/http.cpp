#include "http.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "evhttp.h"

extern lua_State *GlobalL;

namespace pack_http {
	using namespace std;
	/*
	** Translation Table as described in RFC1113
	*/
	static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

	/*
	** Translation Table to decode (created by author)
	*/
	static const char cd64[]="|$$$}rstuvwxyz{$$$$$$$>?@ABCDEFGHIJKLMNOPQRSTUVW$$$$$$XYZ[\\]^_`abcdefghijklmnopq";

	/*
	** returnable errors
	**
	** Error codes returned to the operating system.
	**
	*/
#define B64_SYNTAX_ERROR        1
#define B64_FILE_ERROR          2
#define B64_FILE_IO_ERROR       3
#define B64_ERROR_OUT_CLOSE     4
#define B64_LINE_SIZE_TO_MIN    5
#define B64_SYNTAX_TOOMANYARGS  6

	/*
	** encodeblock
	**
	** encode 3 8-bit binary bytes as 4 '6-bit' characters
	*/
	static void encodeblock( unsigned char *in, unsigned char *out, int len )
	{
	    out[0] = (unsigned char) cb64[ (int)(in[0] >> 2) ];
	    out[1] = (unsigned char) cb64[ (int)(((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4)) ];
	    out[2] = (unsigned char) (len > 1 ? cb64[ (int)(((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6)) ] : '=');
	    out[3] = (unsigned char) (len > 2 ? cb64[ (int)(in[2] & 0x3f) ] : '=');
	}

	/*
	** encode
	**
	** base64 encode a stream adding padding and line breaks as per spec.
	*/

#define B64_DEF_LINE_SIZE   72
#define B64_MIN_LINE_SIZE    4

	static int lbase64_encode(lua_State *L)
	{
		unsigned char *infile = (unsigned char *)(lua_tostring(L, 1));
		if (!infile)
		{
			return 0;
		}
		int in_size = lua_tonumber(L, 2);

		int cur_idx = 0;
		unsigned char in[3];
		unsigned char out[4];
		int i, len, blocksout = 0;

		string out_str;

		*in = (unsigned char) 0;
		*out = (unsigned char) 0;
		while (cur_idx < in_size)
		{
			len = 0;
			for( i = 0; i < 3; i++ ) 
			{
				if (cur_idx < in_size)
				{
					in[i] = infile[cur_idx];
					cur_idx ++;
					len++;
				}
				else
				{
					in[i] = (unsigned char) 0;
				}
			}
			if( len > 0 ) 
			{
				encodeblock( in, out, len );
				for( i = 0; i < 4; i++ ) 
				{
					out_str.push_back(out[i]);
				}
				blocksout++;
			}
			if( blocksout >= (B64_DEF_LINE_SIZE/4) || (cur_idx >= in_size)) 
			{
				if( blocksout > 0 ) 
				{
					//out_str.push_back('\r');
					//out_str.push_back('\n');
				}
				blocksout = 0;
			}
		}
		lua_pushstring(L, out_str.c_str());
		return 1;
	}

	struct http_data 
	{
		struct evhttp_request * req;
		struct evhttp_connection * conn;
		union {
			unsigned int i;
			void * p;
		} cb;
	};

	void free_http(struct http_data * hd)
	{
		evhttp_connection_free(hd->conn);
		free(hd);
	}

	void http_cb(struct evhttp_request * req, void *arg)
	{
		int top = lua_gettop(GlobalL);
		struct http_data * hd = (struct http_data *) arg;

		unsigned int cbid = hd->cb.i;
		lua_getglobal(GlobalL, "__G__TRACKBACK__");

		lua_getglobal(GlobalL, "HTTP");
		lua_pushstring(GlobalL, "onHttpCB");
		lua_rawget(GlobalL, -2);

		if (!lua_isfunction(GlobalL, -1)) {
			free_http(hd);
			lua_settop(GlobalL, top);
			return;
		}

		lua_pushnumber(GlobalL, cbid);

		if (req != NULL)
		{
			lua_pushnumber(GlobalL, req->response_code);
			struct evbuffer * eb = req->input_buffer;
			//不返回空串，返回nil，方便脚本层判断
			if ( EVBUFFER_LENGTH(eb) > 0 )
			{
				lua_pushlstring(GlobalL, (char *)EVBUFFER_DATA(eb), EVBUFFER_LENGTH(eb));
			}
			else
			{
				lua_pushnil(GlobalL);
			}
		}
		else
		{
			lua_pushnumber(GlobalL, -1); //http connect time out
			lua_pushnil(GlobalL);
		}

		if (lua_pcall(GlobalL, 3, 0, -6) != 0) 
		{
			printf("call function failed\n");
		}
		free_http(hd);
		lua_settop(GlobalL, top);
	}

	static int lhttp_post(lua_State * L)
	{
		const char * addr = luaL_checkstring(L, 1);
		char * host = "localhost";
		if (!lua_isnil(L, 2))
		{
			host = (char *)(luaL_checkstring(L, 2));
		}
		int port = (int)luaL_checknumber(L, 3);
		size_t len = 0;
		const char * _uri = luaL_checklstring(L, 4, &len);

		unsigned int cbid = lua_tonumber(L, 5);

		struct http_data * hd = (struct http_data *)malloc(sizeof(struct http_data));
		hd->cb.i = cbid;
		
		struct evhttp_request * req = evhttp_request_new(http_cb, (void *)hd);
		hd->req = req;
		hd->req->major = 1;
		hd->req->minor = 1;//HTTP/1.1
		struct evhttp_connection * conn = evhttp_connection_new(addr, port);
		hd->conn = conn;


		// printf("--------lhttp_post------------------\n");
		//add http head
		//http head参数可选，以table方式做为第六个参数传入
		if (lua_istable(L, -1))
		{
			int head_index = lua_gettop(L);
			lua_pushnil(L);
			while( lua_next(L, head_index) )
			{
				const char* value = luaL_checkstring(L, -1);
				const char* key = luaL_checkstring(L, -2);
				//for debug
				// std::cout << key << ":" << value << std::endl;

				// printf("key = %s, value = %s\n", key, value);
				evhttp_add_header(req->output_headers, key, value);
				//remove value
				lua_pop(L, 1);
			}
		}

		int requrl_len = len;
		const char* dataptr = strrchr(_uri, '?');
		if (dataptr)
		{
			requrl_len = dataptr - _uri;
			dataptr += 1;
			evbuffer_unfreeze( req->output_buffer, 0);
			if (evbuffer_add( req->output_buffer, dataptr, strlen(dataptr) ) )
			{
				luaL_error(L, "evbuffer_add failed:%s", dataptr);
				free_http(hd);
				return 0;
			}
		}	

		char reqstr[requrl_len+1];
		strncpy(reqstr, _uri, sizeof(reqstr));
		reqstr[requrl_len] = '\0';

		evhttp_add_header(req->output_headers, "Host", host);

		if (evhttp_make_request(conn, req, EVHTTP_REQ_POST, reqstr)) 
		{
			luaL_error(L, "evhttp request failed:%s %d %s", addr, port, reqstr);
			free_http(hd);
			return 0;
		}

		return 0;
	}

	static int lhttp_get(lua_State * L)
	{
		const char * addr = luaL_checkstring(L, 1);
		char * host = "localhost";
		if (!lua_isnil(L, 2))
		{
			host = (char *)(luaL_checkstring(L, 2));
		}
		int port = (int)luaL_checknumber(L, 3);
		size_t len = 0;
		const char * _uri = luaL_checklstring(L, 4, &len);

		unsigned int cbid = lua_tonumber(L, 5);

		struct http_data * hd = (struct http_data *)malloc(sizeof(struct http_data));
		hd->cb.i = cbid;
		
		struct evhttp_request * req = evhttp_request_new(http_cb, (void *)hd);
		hd->req = req;
		hd->req->major = 1;
		hd->req->minor = 1;//HTTP/1.1
		struct evhttp_connection * conn = evhttp_connection_new(addr, port);
		hd->conn = conn;

		//add http head
		//http head参数可选，以table方式做为第六个参数传入
		if (lua_istable(L, -1))
		{
			int head_index = lua_gettop(L);
			lua_pushnil(L);
			while( lua_next(L, head_index) )
			{
				const char* value = luaL_checkstring(L, -1);
				const char* key = luaL_checkstring(L, -2);
				//for debug
				//std::cout << key << ":" << value << std::endl;
				evhttp_add_header(req->output_headers, key, value);
				//remove value
				lua_pop(L, 1);
			}
		}

		int requrl_len = len;
		char reqstr[requrl_len+1];
		strncpy(reqstr, _uri, sizeof(reqstr));
		reqstr[requrl_len] = '\0';

		evhttp_add_header(req->output_headers, "Host", host);
		if (evhttp_make_request(conn, req, EVHTTP_REQ_GET, reqstr)) 
		{
			luaL_error(L, "evhttp request failed:%s %d %s", addr, port, reqstr);
			free_http(hd);
			return 0;
		}

		return 0;
	}

	static int lencode_uri(lua_State *L)
	{
		const char *org_str = lua_tostring(L, 1);
		if (!org_str)
		{
			return 0;
		}
		char *encode_str = evhttp_encode_uri(org_str);
		lua_pushstring(L, encode_str);
		free(encode_str);
		return 1;
	}

	static int ldecode_uri(lua_State *L)
	{
		const char *org_str = lua_tostring(L, 1);
		if (!org_str)
		{
			return 0;
		}
		char *decode_str = evhttp_decode_uri(org_str);
		lua_pushstring(L, decode_str);
		free(decode_str);
		return 1;
	}

	const luaL_reg http_func[] =
	{
		{"encode_uri", lencode_uri},
		{"decode_uri", ldecode_uri},
		{"http_get", lhttp_get},
		{"http_post", lhttp_post},
		{"base64_encode", lbase64_encode},
		{NULL, NULL}, 
	};

	void luaopen_httplib(lua_State* L)
	{
		luaL_register(L, "lhttp", http_func);
	}
}

