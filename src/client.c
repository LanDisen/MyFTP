#include "client.h"

void client_log(char* msg) {
    printf("[client] ");
    printf("%s\n", msg);
}

int ls(int socketFd, char* args) {
    char dir[MAX_DIR_LENGTH];
    get_token(dir, args);
    if (dir == NULL || strcmp(dir, "")== 0) {
        // dir设置为当前目录
        strcpy(dir, ".");
    }
    struct ftpmsg msg;
    msg.type = CLIENT_LS;
    msg.len = strlen(dir) + 1;
    msg.data = dir;
    send_msg(socketFd, &msg);
    //printf("send_msg\n");
    recv_msg(socketFd, &msg);
    if (msg.type == SUCCESS) {
        if (strcmp(msg.data, "") != 0) {
            printf("%s\n", msg.data);
        }
    } else if (msg.type == FAILURE) {
        printf("failed to list\n");
        return -1;
    } else {
        printf("wrong message type\n");
        return -1;
    }
    return 0;
}

int cd(int socketFd, char* args) {
    if (args == NULL || strcmp(args, "") == 0) {
        return 0;
    }
    char dir[MAX_DIR_LENGTH];
    get_token(dir, args);
    struct ftpmsg msg;
    msg.type = CLIENT_CD;
    msg.len = strlen(dir) + 1;
    msg.data = dir;
    send_msg(socketFd, &msg);
    recv_msg(socketFd, &msg);
    if (msg.type == FAILURE) {
        perror("cd command error\n");
        return -1;
    }
    if (msg.type == SUCCESS) {
        return 0;
    }
    perror("unexpected cd command args\n");
    return -1;
}

// 下载文件
int get(int socketFd, char* args) {
    if (args == NULL || strcmp(args, "") == 0) {
        perror("\"get\" command needs file path\n");
        return -1;
    }
    char filename[MAX_LENGTH]; // 服务端的文件名
    char newname[MAX_LENGTH]; // 保存在本机的新文件名
    strcpy(newname, "");
    struct ftpmsg msg;
    get_token(filename, args);
    if (args != NULL && strcmp(args, "") != 0) {
        get_token(newname, args);
    }
    msg.type = CLIENT_GET;
    msg.len = strlen(filename) + 1;
    msg.data = filename;
    send_msg(socketFd, &msg);
    recv_file(socketFd, newname);
    return 0;
}

int bye(int socketFd) {
    struct ftpmsg msg;
    msg.type = CLIENT_BYE;
    msg.len = sizeof(msg.type);
    send_msg(socketFd, &msg);
    return 0;
}

// 上传文件
int put(int socketFd, char* args) {
    if (args == NULL) {
        perror("\"put\" command needs file path\n");
        return -1;
    }
    //char arg[MAX_LENGTH];
    struct ftpmsg msg;
    msg.type = CLIENT_PUT;
    char filename[MAX_LENGTH]; // 本机的文件名
    char newname[MAX_LENGTH]; // 保存在客户端的新文件名
    strcpy(newname, "");
    get_token(filename, args);
    if ((args != NULL && strcmp(args, "") != 0)) {
        get_token(newname, args);
    }
    msg.data = newname;
    send_msg(socketFd, &msg);
    send_file(socketFd, filename);
    return 0;
}

int start_client() {
    struct sockaddr_in serverAddr;
    int socketFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (socketFd == -1) {
        client_log("failed to create socket");
        return -1;
    }

    memset(&serverAddr, 0, sizeof(struct sockaddr_in));

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(PORT);
    
    if (inet_pton(AF_INET, IP, &serverAddr.sin_addr) <= 0) { 
        client_log("Invalid address/Address not supported");
        return -1;
    }

    if (connect(socketFd, (const struct sockaddr *)&serverAddr, sizeof(struct sockaddr_in)) == -1) {
        client_log("failed to connect");
        close(socketFd);
        client_log("closed");
        return -1;
    }

    // 客户端输入命令
    while (1) {
        printf("MyFTP> ");
        char cmd[MAX_LENGTH];
        input(cmd);
        //fgets(cmd, MAX_LENGTH, stdin);
        char token[MAX_LENGTH];
        get_token(token, cmd);
        if (cmd == NULL) 
            continue;
        if (strcmp(token, "ls") == 0) {
            // 显示文件列表
            ls(socketFd, cmd);
        } else if (strcmp(token, "cd") == 0) {
            // 切换目录
            cd(socketFd, cmd);
        } else if (strcmp(token, "get") == 0) {
            // 下载文件
            get(socketFd, cmd);
        } else if (strcmp(token, "put") == 0) {
            // 上传文件
            put(socketFd, cmd);
        } else if (strcmp(token, "bye") == 0) {
            // 退出FTP程序
            bye(socketFd);
            break;
        }
    }

    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);

    client_log("bye");
    return 0;
}

int main() {
    start_client();
    return 0;
}