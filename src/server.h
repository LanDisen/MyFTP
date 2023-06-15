#ifndef _SERVER_H
#define _SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "ftpmsg.h"

#define PORT 12345
#define BUFFER_SIZE 1024

#define MAX_CLIENT_NUM 5 // 最大连接客户端数量

void server_log(char* msg);

int client_ls(int socketFd, char* dir);

int client_cd(int socketFd, char* dit);

int client_get(int socketFd, char* path);

int client_put(int socketFd, char* newname);

int start_server();

#endif