#include "client.h"

void client_log(char* msg) {
    printf("[client] ");
    printf("%s\n", msg);
}

// 显示本地文件目录列表
int client_ls(char* args) {
    char dir[MAX_DIR_LENGTH];
    strcpy(dir, "");
    get_token(dir, args);
    char* cwd;
    if (dir == NULL || strcmp(dir, "") == 0) {
        // dir设置为当前目录
        strcpy(dir, ".");
    }
    // 保存当前工作路径
    if ((cwd = getcwd(NULL, 0)) == NULL) {
        printf("getcwd error\n");
        return -1;
    }
    DIR* dir_ptr;
    struct dirent* entry;
    char data[MAX_LENGTH];
    if ((dir_ptr = opendir(dir)) == NULL) {
        printf("failed to list in client\n");
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
    printf("%s\n", data);
    // 切换回原工作目录
    if (cwd != NULL && chdir(cwd) == -1) {
        printf("change current working directory error\n");
        return -1;
    }
    closedir(dir_ptr);
    return 0;
}

// 切换本地目录
int client_cd(char* args) {
    if (args == NULL || strcmp(args, "") == 0) {
        return 0;
    }
    char dir[MAX_DIR_LENGTH];
    get_token(dir, args);
    if (chdir(dir) == -1) {
        printf("%s: No such file or directory\n", dir);
        return -1;
    }
    return 0;
}

int server_ls(int socketFd, char* args) {
    char dir[MAX_DIR_LENGTH];
    get_token(dir, args);
    if (dir == NULL || strcmp(dir, "") == 0) {
        // dir设置为当前目录
        strcpy(dir, ".");
    }
    struct ftpmsg msg;
    msg.type = SERVER_LS;
    msg.len = strlen(dir) + 1;
    msg.data = dir;
    send_msg(socketFd, &msg);
    recv_msg(socketFd, &msg);
    if (msg.type == SUCCESS) {
        if (strcmp(msg.data, "") != 0) {
            // 打印服务端文件列表
            printf("%s\n", msg.data);
        }
    } else if (msg.type == FAILURE) {
        printf("%s: No such file or directory\n", dir);
        return -1;
    } else {
        printf("wrong ftp message type\n");
        return -1;
    }
    return 0;
}

int server_cd(int socketFd, char* args) {
    if (args == NULL || strcmp(args, "") == 0) {
        return 0;
    }
    char dir[MAX_DIR_LENGTH];
    get_token(dir, args);
    struct ftpmsg msg;
    msg.type = SERVER_CD;
    msg.len = strlen(dir) + 1;
    msg.data = dir;
    send_msg(socketFd, &msg);
    recv_msg(socketFd, &msg);
    if (msg.type == FAILURE) {
        printf("%s: No such file or directory\n", dir);
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
    msg.type = GET;
    msg.len = strlen(filename) + 1;
    msg.data = filename;
    send_msg(socketFd, &msg);
    recv_file(socketFd, newname);
    return 0;
}

int bye(int socketFd) {
    struct ftpmsg msg;
    msg.type = BYE;
    msg.len = sizeof(msg.type);
    send_msg(socketFd, &msg);
    return 0;
}

// 上传文件
int put(int socketFd, char* args) {
    if (args == NULL) {
        printf("\"put\" command needs a exact file\n");
        return -1;
    }
    //char arg[MAX_LENGTH];
    struct ftpmsg msg;
    msg.type = PUT;
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
        client_log("failed to create client socket");
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
        char token[MAX_LENGTH];
        strcpy(token, "");
        get_token(token, cmd);
        if (token == NULL || strcmp(token, "") == 0) 
            continue;
        if (strcmp(token, "ls") == 0) {
            // 显示服务端文件列表
            server_ls(socketFd, cmd);
        } else if (strcmp(token, "!ls") == 0) {
            // 显示本地文件列表
            client_ls(cmd);
        } else if (strcmp(token, "cd") == 0) {
            // 切换服务端目录
            server_cd(socketFd, cmd);
        } else if (strcmp(token, "!cd") == 0) {
            // 切换本地目录
            client_cd(cmd);
        } else if (strcmp(token, "get") == 0) {
            // 下载文件
            get(socketFd, cmd);
        } else if (strcmp(token, "put") == 0) {
            // 上传文件
            put(socketFd, cmd);
        } else if (strcmp(token, "bye") == 0 || strcmp(token, "exit") == 0) {
            // 退出FTP程序
            bye(socketFd);
            break;
        } else {
            printf("command '%s' not found\n", token);
        }
        strcpy(cmd, "");
        strcpy(token, "");
    }

    shutdown(socketFd, SHUT_RDWR);
    close(socketFd);
    return 0;
}

int main() {
    start_client();
    return 0;
}