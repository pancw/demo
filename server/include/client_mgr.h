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

	const unsigned char StatusReadHeader = 1;
	const unsigned char StatusReadbody = 2;

	union cb_user_data {
		unsigned int vfd;
		void *p;
	};

class Client 
{
public:
	enum { header_length = 2 };
	enum { max_body_length = 8192 }; // max:256*256+256

	Client(int fd, unsigned int vfd, struct event_base *evBase);
	~Client();

	char write_msg[header_length + max_body_length];
	char read_msg[header_length + max_body_length];
	size_t m_needByteCnt;

	int get_fd(){
		return fd;
	}

	unsigned char get_readStatus(){
		return this->m_readStatus;
	}
	
	void set_readStatus(unsigned char status){
		this->m_readStatus = status;
	}

	bool do_write(const char* line, size_t size){
		memcpy(this->write_msg + header_length, line, size);                                  
		// encode header
		this->write_msg[0] = (unsigned char)(size % 256);
		this->write_msg[1] = (unsigned char)(size / 256);

		bufferevent_write(this->bev, this->write_msg, header_length + size);    
		return true;
	}

	void do_read(struct bufferevent* bufev);

	struct bufferevent * get_bev(){
		return this->bev;
	}

private:
	struct bufferevent *bev;
	unsigned int vfd;
	int fd;
	std::string ip;
	unsigned char m_readStatus;
};

extern std::map<unsigned int, Client*> allClients;
extern void newUserConnect(struct evconnlistener *listener, evutil_socket_t fd, struct sockaddr *sock, int socklen, void *user_data);
extern void release();

}

#endif
