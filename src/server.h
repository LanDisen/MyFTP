#ifndef _SERVER_H
#define _SERVER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include "ftpmsg.h"

#define PORT 12345
#define BUFFER_SIZE 1024

#define MAX_CLIENT_NUM 5 // 最大连接客户端数量

void server_log(char* msg);

int server_ls(int socketFd, char* dir);

int server_cd(int socketFd, char* dir);

int get(int socketFd, char* path);

int put(int socketFd, char* newname);

int start_server();

#endif