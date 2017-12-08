/*
 * client.h
 *
 *  Created on: Nov 10, 2017
 *      Author: alex
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <iostream>
#include <string>
#include "utils.h"
//#include <arpa/inet.h>


using namespace std;

class Client {
private:
	int port;
	int sockfd;
	char buffer[8192];
public:
	Client() {
		port = 80808;
		sockfd = -1;
	}
	Client(int p) {
		port = p;
		sockfd = -1;
	}
	virtual ~Client() {}
	void error(const char *msg)
	{
	    perror(msg);
	    exit(0);
	}
	void closeSocket(){
		if(sockfd != -1){
			close(sockfd);
			sockfd = -1;
		}
	}
	void openSocket(string host){
//		cout << "f:" << host << port << endl;
		if(sockfd == -1){
			struct hostent *server;
			struct sockaddr_in serv_addr;
			sockfd = socket(AF_INET, SOCK_STREAM, 0);
//			serv_addr.sin_addr.s_addr = inet_addr(host.c_str());
			if (sockfd < 0)
				error("ERROR opening socket");
			server = gethostbyname(host.c_str());
			if (server == NULL) {
				fprintf(stderr,"ERROR, no such host\n");
				exit(0);
			}
			bzero((char *) &serv_addr, sizeof(serv_addr));
			serv_addr.sin_family = AF_INET;
			bcopy((char *)server->h_addr,
				 (char *)&serv_addr.sin_addr.s_addr,
				 server->h_length);
			serv_addr.sin_port = htons(port);
			if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
				error("ERROR connecting");
		}
	}
	void sendMessage(string mes) {
		int n;
		bzero(buffer,8192);
		mes.copy(buffer, mes.length(), 0);
		n = write(sockfd,buffer,strlen(buffer));
		if (n < 0)
			 error("ERROR writing to socket");
		bzero(buffer,8192);
		n = read(sockfd,buffer,8192);
		if (n < 0)
			 error("ERROR reading from socket");
		printf("%s\n",buffer);
		utils::print("==========================================");
	}
};

#endif /* CLIENT_H_ */
