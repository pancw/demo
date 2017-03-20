#include "engine_base.h"
#include "client_mgr.h"

extern lua_State* GlobalL;
extern int error_fun(lua_State* state);

namespace engine_base {

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

	bool clientListen(int port)
	{
		struct sockaddr_in sin;
		memset(&sin, 0, sizeof(struct sockaddr_in));
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
	
		clientListener = evconnlistener_new_bind(base, mgr::newUserConnect, base, LEV_OPT_REUSEABLE|LEV_OPT_CLOSE_ON_FREE, -1, (struct sockaddr*)&sin, sizeof(struct sockaddr_in));
	
		if (!clientListener)
		{
			fprintf(stderr, "Could not create a clientListener!\n");	
			return false;
		}	
		return true;
	}	

	static void timeout_cb(evutil_socket_t fd, short event, void *arg)
	{
		//printf("sec tick\n");
		//struct timeval now = evutil_gettimeofday(&now, NULL);
		
		int top = lua_gettop(GlobalL);
		lua_pushcclosure(GlobalL, error_fun, 0);
		lua_getglobal(GlobalL, "Tick");
		int result = lua_pcall(GlobalL, 0, 0, -2);
		if (result) {
			printf("[lua-call(%d)]: %s\n", 1, lua_tostring(GlobalL, -1));
		}
		lua_settop(GlobalL, top);
	}	

	void loop()
	{
		struct timeval tv;
		evutil_timerclear(&tv);
		tv.tv_sec = 3;

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
