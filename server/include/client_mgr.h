#ifndef __CLIENT_H__
#define __CLIENT_H__

#include<netinet/in.h>    
#include<sys/socket.h>    
#include<unistd.h>    
#include<stdio.h>    
#include<string.h>    
#include<event2/event.h>    
#include<event2/listener.h>    
#include<event2/bufferevent.h>    
#include<event2/thread.h>    
#include <event2/event-config.h>

#include <signal.h>
#include <stdlib.h>
#include <errno.h>
#include <event.h>
#include <iostream>
#include <arpa/inet.h>
#include <map>
#include <cstdlib>

namespace mgr {

	union cb_user_data {
		unsigned int vfd;
		void *p;
	};

//class Client 
//{
//
//};
//
//extern std::map<unsigned int, Client*> allClients;
extern void newUserConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *user_data);


}

#endif
