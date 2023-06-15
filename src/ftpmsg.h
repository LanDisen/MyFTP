#ifndef _FTPMSG_H
#define _FTPMSG_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "utils.h"

#define BUFFER_SIZE 1024

#define MAX_LENGTH 256
#define MAX_CMD_LENGTH 256
#define MAX_FILE_LENGTH 256
#define MAX_DIR_LENGTH 64

enum ftpmsg_type {
    DEFAULT,

    CLIENT_LS, // 客户端请求显示文件列表
    CLIENT_CD, // 客户端cd命令
    CLIENT_PUT,  // 客户端上传文件
    CLIENT_GET,  // 客户端下载文件
    CLIENT_EXIT, // 客户端退出

    FILE_NAME,
    FILE_MODE, // struct stat的st_mode
    FILE_SIZE,
    FILE_DATA,
    FILE_OVER,

    SUCCESS,
    FAILURE
};

struct ftpmsg {
    enum ftpmsg_type type;
    char* data;
    int len;
};

int send_msg(int socketFd, struct ftpmsg *msg);
int recv_msg(int socketFd, struct ftpmsg *msg);
int send_file(int socketFd, char *path);
int recv_file(int socketFd, char* newname);

#endif