#include "client_mgr.h"

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

	void Client::do_read(struct bufferevent* bufev)
	{
		size_t dataLen = EVBUFFER_LENGTH(EVBUFFER_INPUT(bufev));	

		char msg[1024];
		size_t len = bufferevent_read(bufev, msg, sizeof(msg)-1);
		msg[len] = '\0';
		printf("len:%d, msg:%s", dataLen, msg);

		char reply[] = "I has read your data!";
		bufferevent_write(bufev, reply, strlen(reply));
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

	Client::Client(int fd, unsigned int vfd, struct event_base *evBase){

		this->vfd = vfd;
		this->fd = fd;
		this->m_readStatus = 1;
		this->m_needByteCnt = header_length;
		this->bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);

		cb_user_data usd;
		usd.vfd = vfd;
		bufferevent_setcb(this->bev,  socket_read_cb, NULL, socket_event_cb, usd.p);
		bufferevent_enable(this->bev, EV_READ|EV_WRITE|EV_PERSIST);
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
