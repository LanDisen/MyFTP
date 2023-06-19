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
    SERVER_LS, // 请求服务端显示文件列表
    SERVER_CD, // 请求服务端切换目录
    PUT, // 上传文件
    GET, // 下载文件
    BYE, // 客户端退出
    FILE_NAME, // 文件名
    FILE_MODE, // struct stat的st_mode
    FILE_SIZE, // 文件大小
    FILE_DATA, // 文件数据
    FILE_OVER, // 文件传输完毕
    SUCCESS, // 消息成功接收
    FAILURE  // 消息成功接收
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