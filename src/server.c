#include "server.h"

void server_log(char* msg) {
    printf("[server] ");
    printf("%s\n", msg);
}

int client_ls(int serverSocketFd, char* dir) {
    DIR* dir_ptr;
    struct dirent* entry;
    char data[MAX_LENGTH];
    struct ftpmsg msg;
    
    if ((dir_ptr = opendir(dir)) == NULL) {
        printf("failed to list");
        msg.type = FAILURE;
        msg.len = 0;
        msg.data = NULL;
        send_msg(serverSocketFd, &msg);
    }
    chdir(dir);
    strcpy(data, "");
    while ((entry = readdir(dir_ptr)) != NULL) {
        // 跳过当前目录和父目录
        if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
            continue;
        strcat(data, entry->d_name);
        strcat(data, "  ");
    }
    msg.type = SUCCESS;
    msg.len = strlen(data) + 1;
    msg.data = data;
    send_msg(serverSocketFd, &msg);
    server_log("list successfully");
    closedir(dir_ptr);
    return 0;
}

int client_cd(int socketFd, char* dir) {
    struct ftpmsg msg;
    if (chdir(dir) == -1) {
        perror("cd command error\n");
        msg.type = FAILURE;
        send_msg(socketFd, &msg);
        return -1;
    }
    char log[MAX_LENGTH];
    sprintf(log, "client request cd %s", dir);
    server_log(log);
    msg.type = SUCCESS;
    send_msg(socketFd, &msg);
    return 0;
}

int client_get(int socketFd, char* path) {
    return send_file(socketFd, path);
}

int client_put(int socketFd) {
    return recv_file(socketFd);
}

int start_server() {
    struct sockaddr_in serverSocketAddr;
    int serverSocketFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    //char buf[BUFFER_SIZE];

    if (serverSocketFd == -1) {
        server_log("failed to create socket");
        return -1;
    }

    memset(&serverSocketAddr, 0, sizeof(struct sockaddr_in));

    serverSocketAddr.sin_family = AF_INET;
    serverSocketAddr.sin_port = htons(PORT);
    serverSocketAddr.sin_family = INADDR_ANY;

    if (bind(serverSocketFd, (const struct sockaddr*)&serverSocketAddr, sizeof(struct sockaddr_in)) == -1) {
        server_log("failed to bind");
        close(serverSocketFd);
        server_log("closed");
        return -1;
    }

    if (listen(serverSocketFd, 3) == -1) {
        server_log("failed to listen");
        close(serverSocketFd);
        server_log("closed");
        return -1;
    }

    server_log("started");

    int clientNum = 0;
    struct ftpmsg msg;
    //char cmd[MAX_CMD_LENGTH];
    int clients[MAX_CLIENT_NUM]; // 多客户端的socketFd

    while (1) {
        if (clientNum == 0) {
            int clientSocketFd = accept(serverSocketFd, NULL, NULL);
            if (clientSocketFd == -1) {
                perror("failed to accpet\n");
                close(serverSocketFd);
                server_log("closed");
                return -1;
            } else {
                clients[clientNum++] = clientSocketFd;
                server_log("a client joined");
            }
        }

        // TODO 暂时只实现了单客户端，后续实现多客户端连接
        if (clientNum > 0) {
            recv_msg(clients[0], &msg);
            switch (msg.type) {
                case CLIENT_LS:
                    client_ls(clients[0], msg.data); break;
                case CLIENT_CD:
                    client_cd(clients[0], msg.data); break;
                case CLIENT_GET:
                    client_get(clients[0], msg.data); break;
                case CLIENT_PUT:
                    client_put(clients[0]); break;
                default:
                    break;
            }
            msg.type = DEFAULT;
        }

       
        
    }
    close(serverSocketFd);
    server_log("closed");
    return 0;
}

int main() {
    start_server();
    return 0;
}
