#include "server.h"

void server_log(char* msg) {
    printf("[server] ");
    printf("%s\n", msg);
}

int server_ls(int socketFd, char* dir) {
    DIR* dir_ptr;
    struct dirent* entry;
    char data[MAX_LENGTH];
    struct ftpmsg msg;
    char *cwd;
    if ((dir_ptr = opendir(dir)) == NULL) {
        printf("failed to list for client %d\n", socketFd);
        msg.type = FAILURE;
        msg.len = 0;
        msg.data = NULL;
        send_msg(socketFd, &msg);
    }
    // 保存当前工作路径
    if ((cwd = getcwd(NULL, 0)) == NULL) {
        printf("getcwd error\n");
        return -1;
    }
    if (chdir(dir) == -1) {
        printf("%s: No such file or directory\n", dir);
        return -1;
    }
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
    send_msg(socketFd, &msg);
    // 切换回原工作目录
    if (cwd != NULL && chdir(cwd) == -1) {
        printf("failed to change current working directory\n");
        return -1;
    }
    char log[MAX_LENGTH];
    sprintf(log, "list for client %d successfully", socketFd);
    server_log(log);
    closedir(dir_ptr);
    return 0;
}

int server_cd(int socketFd, char* dir) {
    struct ftpmsg msg;
    if (chdir(dir) == -1) {
        printf("failed to change current working directory\n");
        msg.type = FAILURE;
        send_msg(socketFd, &msg);
        return -1;
    }
    char log[MAX_LENGTH];
    sprintf(log, "client %d request change directory %s", socketFd, dir);
    server_log(log);
    msg.type = SUCCESS;
    send_msg(socketFd, &msg);
    return 0;
}

int get(int socketFd, char* path) {
    char log[MAX_LENGTH];
    sprintf(log, "client %d request get file %s", socketFd,  path);
    server_log(log);
    return send_file(socketFd, path);
}

int put(int socketFd, char* newname) {
    char log[MAX_LENGTH];
    if (newname != NULL) {
        sprintf(log, "client %d request get file %s", socketFd, newname);
    } else {
        sprintf(log, "client %d request get file", socketFd);
    }
    server_log(log);
    return recv_file(socketFd, newname);
}

// 线程函数，开启一个新的线程服务客户端
void* serve(void* args) {
    int clientSocketFd = *((int*)args);
    while (1) {
        struct ftpmsg msg;
        recv_msg(clientSocketFd, &msg);
        switch (msg.type) {
            case SERVER_LS:
                server_ls(clientSocketFd, msg.data); break;
            case SERVER_CD:
                server_cd(clientSocketFd, msg.data); break;
            case GET:
                get(clientSocketFd, msg.data); break;
            case PUT:
                put(clientSocketFd, msg.data); break;
            case BYE: {
                char log[MAX_LENGTH];
                sprintf(log, "a client %d exit", clientSocketFd);
                server_log(log);
                break;
            }
            default:
                printf("unknown ftp message type\n");
                break;
        }
        msg.type = DEFAULT;
    }
    pthread_exit(0);
}

int start_server() {
    struct sockaddr_in serverSocketAddr;
    int serverSocketFd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (serverSocketFd == -1) {
        server_log("failed to create server's socket");
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

    while (1) {
        // 主线程等待新的客户端连接
        int clientSocketFd = accept(serverSocketFd, NULL, NULL);
        if (clientSocketFd == -1) {
            perror("failed to accpet\n");
            close(serverSocketFd);
            server_log("closed");
            return -1;
        } else {
            // 有新的客户端连接到服务器，创建子线程进行服务
            pthread_t clientThread;
            if (pthread_create(&clientThread, NULL, serve, &clientSocketFd) != 0) {
                printf("failed to creant a client %d's thread\n", clientSocketFd);
                return -1;
            }
            char log[MAX_LENGTH];
            sprintf(log, "a client %d joined", clientSocketFd);
            server_log(log);
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
