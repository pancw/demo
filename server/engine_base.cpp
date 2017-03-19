#include "engine_base.h"

namespace engine_base {

	unsigned int globalVfd = 1;

	struct event_base *base = NULL;
	struct evconnlistener *clientListener;
	struct event *signal_event = NULL;
	struct event *timeout = NULL;

	static void signal_cb(evutil_socket_t fd, short events, void *arg)
	{
		printf("%s:got signal %d\n", __func__, fd);

		// lua TODO
		event_base_loopbreak(base);
	}

	void init()
	{
		base = event_base_new();	
		if (!base)
		{
			fprintf(stderr, "Coult not initialize libevent!\n");
			return;
		}

		signal_event = evsignal_new(base, SIGINT, signal_cb, NULL);
		if (!signal_event || event_add(signal_event, NULL)<0) {
			fprintf(stderr, "Could not create/add a signal event!\n");
			return;
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

	static void clientListenCb(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *user_data)
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

	bool clientListen(int port)
	{
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
	
		clientListener = evconnlistener_new_bind(base, clientListenCb, base, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	
		if (!clientListener)
		{
			fprintf(stderr, "Could not create a clientListener!\n");	
			return false;
		}	
		return true;
	}	

	static void timeout_cb(evutil_socket_t fd, short event, void *arg)
	{
		printf("sec tick\n");
		//struct timeval now = evutil_gettimeofday(&now, NULL);
	}	

	void loop()
	{
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 1;

		//timeout = evtimer_new(base, timeout_cb, NULL);
		timeout = event_new(base, -1, EV_PERSIST, timeout_cb, NULL);
		event_add(timeout, &tv);

		event_base_dispatch(base);
	}
	
	void release()
	{
		evconnlistener_free(clientListener);
		event_free(signal_event);
		event_free(timeout);
		event_base_free(base);
	}
}
