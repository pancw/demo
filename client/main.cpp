#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<errno.h>
#include<unistd.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>

#include<event.h>
#include<event2/bufferevent.h>
#include<event2/buffer.h>
#include<event2/util.h>

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

int tcp_connect_server(const char* server_ip, int port);
void cmd_msg_cb(int fd, short events, void* arg);
void server_msg_cb(struct bufferevent* bev, void* arg);
void event_cb(struct bufferevent *bev, short event, void *arg);

lua_State *GlobalL;

static void InitLuaLib()
{
	GlobalL = luaL_newstate();
	lua_gc(GlobalL, LUA_GCSTOP, 0);	
	luaL_openlibs(GlobalL);
	lua_gc(GlobalL, LUA_GCRESTART, 0);
}

int main(int argc, char** argv)
{
	if( argc < 3 )
	{
		//两个参数依次是服务器端的IP地址、端口号
		printf("please input 2 parameter\n");
		return -1;
	}

	// lua
	if (chdir("../logic") == -1)
	{
		fprintf(stderr, "bad logic path to dir:%s\n", "../logic");
		return 1;
	}

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

	struct event_base *base = event_base_new();

	struct bufferevent* bev = bufferevent_socket_new(base, -1,
													 BEV_OPT_CLOSE_ON_FREE);

	//监听终端输入事件
	struct event* ev_cmd = event_new(base, STDIN_FILENO,
									 EV_READ | EV_PERSIST,
									 cmd_msg_cb, (void*)bev);


	event_add(ev_cmd, NULL);

	struct sockaddr_in server_addr;

	memset(&server_addr, 0, sizeof(server_addr) );

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(atoi(argv[2]));
	inet_aton(argv[1], &server_addr.sin_addr);

	bufferevent_socket_connect(bev, (struct sockaddr *)&server_addr,
							   sizeof(server_addr));


	bufferevent_setcb(bev, server_msg_cb, NULL, event_cb, (void*)ev_cmd);
	bufferevent_enable(bev, EV_READ | EV_PERSIST);



	event_base_dispatch(base);

	printf("finished \n");
	return 0;
}

void cmd_msg_cb(int fd, short events, void* arg)
{
	char msg1[1024];
	char msg[1024];

	int ret = read(fd, msg1, sizeof(msg1));
	if( ret < 0 )
	{
		perror("read fail ");
		exit(1);
	}

	char data[] = "this is a test data.\
=======================================!!!!==================================================================================================================================================\
==========================================oooo===============================================================================================================================================end";
	unsigned int len = strlen(data);
	memcpy(msg+2, data, strlen(data));	

	msg[0] = (unsigned char)(len % 256);
	msg[1] = (unsigned char)(len / 256);

	struct bufferevent* bev = (struct bufferevent*)arg;
	bufferevent_write(bev, msg, 2+len);
	printf("--------write\n");
}


void server_msg_cb(struct bufferevent* bev, void* arg)
{
	char msg[10000];

	size_t len = bufferevent_read(bev, msg, 2);
	size_t needByte = (unsigned char)msg[0] + ((unsigned char)msg[1])*256;
	len = bufferevent_read(bev, msg, needByte);

	msg[len] = '\0';

	printf("recv:[%s]\n", msg);
}


void event_cb(struct bufferevent *bev, short event, void *arg)
{

	if (event & BEV_EVENT_EOF)
		printf("connection closed\n");
	else if (event & BEV_EVENT_ERROR)
		printf("some other error\n");
	else if( event & BEV_EVENT_CONNECTED)
	{
		printf("the client has connected to server\n");
		return ;
	}

	//这将自动close套接字和free读写缓冲区
	bufferevent_free(bev);

	struct event *ev = (struct event*)arg;
	event_free(ev);
}
