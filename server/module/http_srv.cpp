#include "http_srv.hpp"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdarg.h>
#include "evhttp.h"

extern lua_State *GlobalL;   //lua的解释器对象
namespace pack_httpsrv {

	struct evhttp *httpd = NULL;
	void httpd_handler(struct evhttp_request *req, void *arg) 
	{
		evhttp_cmd_type cmd_type = evhttp_request_get_command(req);
		if (cmd_type != EVHTTP_REQ_GET)
		{
			evhttp_send_reply(req, 199, "ERROR", NULL);
			return;
		}

		int top = lua_gettop(GlobalL);
		lua_getglobal(GlobalL, "__G__TRACKBACK__");
		int trace_idx = lua_gettop(GlobalL);
		lua_getglobal(GlobalL, "onHttpRequest");
		lua_newtable(GlobalL); // request info tbl

		struct evkeyvalq *all_header;
		struct evkeyval *header_kv;

		all_header = evhttp_request_get_input_headers(req);
		for (header_kv = all_header->tqh_first; header_kv; header_kv = header_kv->next.tqe_next) 
		{
			lua_pushstring(GlobalL, header_kv->key);
			lua_pushstring(GlobalL, header_kv->value);
			lua_rawset(GlobalL,  -3);
		}

		const char *uri;
		uri = evhttp_request_uri(req);
		// char *decoded_uri;
		// decoded_uri = evhttp_decode_uri(uri); // return new allocate string; need free

		struct evkeyvalq params;
		struct evkeyval *param_kv;
		
		evhttp_parse_query(uri, &params);

		lua_newtable(GlobalL); // request param kv tbl
		for (param_kv = params.tqh_first; param_kv; param_kv = param_kv->next.tqe_next) 
		{
			lua_pushstring(GlobalL, param_kv->key);
			lua_pushstring(GlobalL, param_kv->value);
			lua_rawset(GlobalL,  -3);
		}
		// free(decoded_uri);

		if (!lua_isfunction(GlobalL, -3))
		{
			lua_settop(GlobalL, top);
			return;
		}

		if (lua_pcall(GlobalL, 2, 3, trace_idx) != 0)
		{
			evhttp_send_reply(req, 200, "ERROR", NULL);
			lua_settop(GlobalL, top);
			return;
		}
		else
		{
			int ret_code = lua_tonumber(GlobalL, -3);
			const char *msg = lua_tostring(GlobalL, -2);
			const char *body = lua_tostring(GlobalL, -1);

			evhttp_add_header(req->output_headers, "Server", "gameServer");
			evhttp_add_header(req->output_headers, "Content-Type", "text/plain; charset=UTF-8");
			evhttp_add_header(req->output_headers, "Connection", "close");
			//输出的内容
			struct evbuffer *buf;
			buf = evbuffer_new();
			evbuffer_add_printf(buf, "%s\n", body);
			evhttp_send_reply(req, ret_code, msg, buf);
			evbuffer_free(buf);
		}
		lua_settop(GlobalL, top);
	}

	void startHttpSrv(int http_port)
	{
		if (httpd)
		{
			return;
		}
		httpd = evhttp_start("0.0.0.0", http_port);
		evhttp_set_timeout(httpd, 60);
		evhttp_set_gencb(httpd, httpd_handler, NULL);
		printf("Accepting http req on port %d \n", http_port);
	}

	void release()
	{
		if (!httpd)
		{
			return;
		}
		evhttp_free(httpd);
		httpd = NULL;
	}
}

