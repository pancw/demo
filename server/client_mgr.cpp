#include "client_mgr.h"

extern lua_State* GlobalL;
extern int error_fun(lua_State* state);

namespace mgr {

	unsigned int globalVfd = 1;
	std::map<unsigned int, Client*> allClients;

	static Client* queryClient(unsigned int vfd)
	{
		std::map<unsigned int, Client*>::iterator it = allClients.find(vfd);
		if (it == allClients.end())
		{
			return NULL;
		}
		return it->second;
	}

	static void tryReleaseClient(unsigned int vfd)
	{
		Client* c = queryClient(vfd);
		if (c)
		{
			allClients.erase(vfd);
			delete c;
		}	
		else
		{
			fprintf(stderr, "tryReleaseClient error!vfd:%d\n", vfd);
		}
		// TODO LUA
	}

	static void socket_event_cb(struct bufferevent *bev, short events, void *arg)
	{
		cb_user_data usd;
		usd.p = arg;
		unsigned int vfd = usd.vfd;

		if (events & BEV_EVENT_EOF)
			printf("vfd:%d connection close.\n", vfd);
		else if (events & BEV_EVENT_ERROR)
			printf("vfd:%d some other error!\n", vfd);

		tryReleaseClient(vfd);
	}

	static void handle_event(unsigned vfd, const char* msg, size_t len)
	{
		int top = lua_gettop(GlobalL);

		lua_pushcclosure(GlobalL, error_fun, 0);
		lua_getglobal(GlobalL, "handle_event");
		lua_pushnumber(GlobalL, vfd);
		lua_pushlstring(GlobalL, msg, len);
		
		int result = lua_pcall(GlobalL, 2, 0, -2-2);
		if (result)
		{
			printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
		}
		lua_settop(GlobalL, top);
	}

	void Client::do_read(struct bufferevent* bufev)
	{
		//size_t dataLen = EVBUFFER_LENGTH(EVBUFFER_INPUT(bufev));	
		//char msg[20000];
		//size_t len = bufferevent_read(bufev, msg, sizeof(msg)-1);
		//msg[len] = '\0';
		//printf("len:%d, msg:%s\n", dataLen, msg);

		//char reply[] = "I has read your data!";
		//bufferevent_write(bufev, reply, strlen(reply));


		//size_t len = bufferevent_read(bufev, this->read_msg, mgr::Client::header_length);
		//this->m_needByteCnt = (unsigned char)this->read_msg[0] + (unsigned char)(this->read_msg[1]*256);

		//printf("dataLen:%d,len:%d\n", dataLen, len);

		//len = bufferevent_read(bufev, this->read_msg, this->m_needByteCnt);

		//printf("=====dataLen:%d,len:%d\n", dataLen, len);
		//this->read_msg[len] = '\0';
		//printf("read_msg:%s\n", this->read_msg);

		size_t dataLen = EVBUFFER_LENGTH(EVBUFFER_INPUT(bufev)); 
		while (dataLen > 0)
		{
			if (this->get_readStatus() == StatusReadHeader)
			{
				size_t len = bufferevent_read(bufev, this->read_msg, mgr::Client::header_length);
				if (len != mgr::Client::header_length)
				{
					fprintf(stderr, "read header error!\n");
					tryReleaseClient(this->getVfd());
					return;
				}

				// decode header
				this->m_needByteCnt = (unsigned char)this->read_msg[0] + (unsigned char)(this->read_msg[1]*256);

				dataLen -= len;
				this->set_readStatus(StatusReadbody);
			}
			else if (this->get_readStatus() == StatusReadbody)
			{
				size_t len = bufferevent_read(bufev, this->read_msg, this->m_needByteCnt);
				if (len != this->m_needByteCnt)
				{
					fprintf(stderr, "read body error!\n");
					tryReleaseClient(this->getVfd());
					return;
				}

				dataLen -= len;
				this->set_readStatus(StatusReadHeader);
				handle_event(this->getVfd(), this->read_msg, len);
			}	
		}
	}

	static void socket_read_cb(struct bufferevent *bev, void *arg)
	{
		cb_user_data usd;
		usd.p = arg;
		unsigned int vfd = usd.vfd;

		Client* client = queryClient(vfd);	
		if (client)
		{
			client->do_read(bev);
		}
		else
		{
			fprintf(stderr, "read data no client!\n");
		}
	}

	void newUserConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *user_data)
	{
		unsigned vfd = 	globalVfd++;
		printf("accept a client. globalVfd=%d, nowVfd=%d\n", globalVfd, vfd);
	
		struct event_base *evBase = (struct event_base*)user_data;

		Client* client = new Client(fd, vfd, evBase);
		if (!allClients.insert(std::make_pair(vfd, client)).second)
		{
			close(fd);
			delete client;
			fprintf(stderr, "client insert error!\n");
			return;
		}
	}

	static int send(lua_State* L)
	{
		size_t size;
		unsigned vfd = lua_tonumber(L, 1);
		const char* msg = luaL_checklstring(L, 2, &size);
		Client* c = queryClient(vfd);
		if (c)
		{
			lua_pushboolean(L, c->do_write(msg, size));
		}
		else
		{
			lua_pushboolean(L, false);
			fprintf(stderr, "send data no client!\n");
		}
		return 1;
	}

	const luaL_reg netlib[] =
	{	
		{"send", send},
		{NULL, NULL},
	};

	void luaopen_netlib(lua_State* L)
	{
		luaL_register(L, "lnetlib", netlib);	
	}

	Client::Client(int fd, unsigned int vfd, struct event_base *evBase){

		this->vfd = vfd;
		this->fd = fd;
		this->m_readStatus = 1;
		this->m_needByteCnt = header_length;
		this->bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);

		cb_user_data usd;
		usd.vfd = vfd;
		bufferevent_setcb(this->bev,  socket_read_cb, NULL, socket_event_cb, usd.p);
		bufferevent_enable(this->bev, EV_READ|EV_WRITE);//|EV_PERSIST);
	}

	Client::~Client()
	{
		//TODO	
		bufferevent_free(this->bev);
		printf("release a client.\n");
	}

	void release()
	{
		std::map<unsigned int, Client*>::iterator it = allClients.begin();				
		for (; it != allClients.end(); it++)
		{
			Client* c = it->second;	
			delete c;
		}
		allClients.clear();
	}
}
