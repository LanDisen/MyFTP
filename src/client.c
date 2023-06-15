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

// 下载文件
int get(int socketFd, char* args) {
    if (args == NULL) {
        perror("\"get\" command needs file path\n");
        return -1;
    }
    char arg[MAX_LENGTH];
    struct ftpmsg msg;
    while (get_token(arg, args) != -1) {
        //printf("token %s\n", arg);
        msg.type = CLIENT_GET;
        msg.len = strlen(arg) + 1;
        msg.data = arg;
        // 告诉服务端要下载文件
        send_msg(socketFd, &msg);
        // 接收文件数据
        recv_file(socketFd);
    }
    return 0;
}

// 上传文件
int put(int socketFd, char* args) {
    if (args == NULL) {
        perror("\"put\" command needs file path\n");
        return -1;
    }
    char arg[MAX_LENGTH];
    struct ftpmsg msg;
    msg.type = CLIENT_PUT;
    while (get_token(arg, args) != -1) {
        send_msg(socketFd, &msg);
        send_file(socketFd, arg);
    }
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

    // char* msg = "hello\n";
    // if (send(socketFd, msg, strlen(msg), 0) == -1) {
    //     client_log("failed to send");
    //     close(socketFd);
    //     client_log("closed");
    //     return -1;
    // }

    //client_log("send successfully");

    // 客户端输入命令
    while (1) {
        printf("MyFTP > ");
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
        } else if (strcmp(token, "get") == 0) {
            // 下载文件
            get(socketFd, cmd);
        } else if (strcmp(token, "put") == 0) {
            // 上传文件
            // TODO DUBUG
            put(socketFd, cmd);
        }
    }

    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);

    client_log("closed");
    return 0;
}

int main() {
    start_client();
    return 0;
}