#include "client_mgr.h"

namespace mgr {

	unsigned int globalVfd = 1;

	static void socket_event_cb(struct bufferevent *bev, short events, void *arg)
	{
		cb_user_data usd;
		usd.p = arg;
		unsigned int vfd = usd.vfd;

		if (events & BEV_EVENT_EOF)
			printf("vfd:%d connection close.\n", vfd);
		else if (events & BEV_EVENT_ERROR)
			printf("vfd:%d some other error!\n", vfd);

		// TODO
		bufferevent_free(bev);
	}

	static void socket_read_cb(struct bufferevent *bev, void *arg)
	{
		cb_user_data usd;
		usd.p = arg;
		unsigned int vfd = usd.vfd;

		size_t dataLen = EVBUFFER_LENGTH(EVBUFFER_INPUT(bev));	

		char msg[1024];
		size_t len = bufferevent_read(bev, msg, sizeof(msg)-1);
		msg[len] = '\0';
		printf("len:%d, msg:%s", dataLen, msg);

		char reply[] = "I has read your data!";
		bufferevent_write(bev, reply, strlen(reply));
	}

	void newUserConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *user_data)
	{
		unsigned vfd = 	globalVfd++;
		printf("accept a client. globalVfd=%d, nowVfd=%d\n", globalVfd, vfd);
	
		struct event_base *evBase = (struct event_base*)user_data;

		struct bufferevent *bev = bufferevent_socket_new(evBase, fd, BEV_OPT_CLOSE_ON_FREE);

		//TODO ip
		//client

		cb_user_data usd;
		usd.vfd = vfd;

		bufferevent_setcb(bev,  socket_read_cb, NULL, socket_event_cb, usd.p);
		bufferevent_enable(bev, EV_READ|EV_PERSIST);
	}

}
