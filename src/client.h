#ifndef _CLIENT_H
#define _CLIENT_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "utils.h"
#include "ftpmsg.h"

#define IP "127.0.0.1"
#define PORT 12345

void client_log(char* msg);

// 显示本地文件目录列表
int client_ls(char* args);

// 切换本地目录
int client_cd(char* args);

// 显示文件列表
int server_ls(int socketFd, char* args);

// 目录切换
int server_cd(int socketFd, char* args);

// 下载文件
int get(int socketFd, char* args);

// 上传文件
int put(int socketFd, char* args);

// 退出MyFTP程序
int bye(int socketFd);

int start_client();

#endif