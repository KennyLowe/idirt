#ifndef _S_SOCKET_H
#define _S_SOCKET_H

int make_service(short unsigned int port, char *my_hostname,
		 int my_hostnamelen, struct hostent **hp,
		 struct sockaddr_in *sin);
#endif
