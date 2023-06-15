#include "ftpmsg.h"

int send_msg(int socketFd, struct ftpmsg *msg) {
    int ret;
    ret = send(socketFd, &msg->type, sizeof(int), 0);
    if (ret == -1) {
        perror("failed to send message\n");
        return -1;
    }

    ret = send(socketFd, &msg->len, sizeof(int), 0);
    if (ret == -1)
    {
        perror("failed to send message\n");
        return -1;
    }

    if (msg->len == 0) {
        return 0;
    }

    int sent_length = 0; // 发送长度
    while (sent_length < msg->len) {
        ret = send(socketFd, msg->data + sent_length, msg->len - sent_length, 0);
        if (ret == -1) {
            perror("failed to send message\n");
            return -1;
        } else {
            sent_length += ret;
        }
    }
    return 0;
}

int recv_msg(int socketFd, struct ftpmsg *msg) {
    int ret;
    ret = recv(socketFd, &msg->type, sizeof(int), 0);
    if (ret == -1) {
        perror("failed to recv message\n");
        return -1;
    }

    ret = recv(socketFd, &msg->len, sizeof(int), 0);
    if (ret == -1) {
        perror("failed to recv message\n");
        return -1;
    }

    if (msg->len == 0) {
        msg->data = NULL;
    } else {
        msg->data = malloc(msg->len);
        if (msg->data == NULL) {
            perror("failed to malloc\n");
            return -1;
        }

        int recv_length = 0;
        while (recv_length < msg->len)  {
            ret = recv(socketFd, msg->data + recv_length, msg->len - recv_length, 0);
            if (ret == -1) {
                perror("failed to recv message\n");
                return -1;
            } else {
                recv_length += ret;
            }
        }
    }
    //printf("recv_data: %s\n", msg->data);
    return 0;
}

int send_file(int socketFd, char *path) {
    char buf[BUFFER_SIZE];
    char filename[MAX_FILE_LENGTH];
    int fd;
    int read_length = 0;
    long sent_length = 0;
    struct stat structStat;
    struct ftpmsg msg;
    // printf("pre open %s\n", path); //debug
    fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("failed to open file\n");
        return -1;
    }

    // 读取文件属性
    if (stat(path, &structStat) == -1) {
        perror("stat error\n");
        return -1;
    }

    // 发送filename
    msg.type = FILE_NAME;
    msg.data = malloc(sizeof(path));
    if (msg.data == NULL) {
        perror("malloc error while sending file\n");
        return -1;
    }
    get_filename(msg.data, path);
    get_filename(filename, path);
    msg.len = strlen(msg.data) + 1;
    send_msg(socketFd, &msg);

    msg.type = FILE_MODE;
    sprintf(msg.data, "%d", structStat.st_mode);
    msg.len = strlen(msg.data) + 1;
    send_msg(socketFd, &msg); 

    msg.type = FILE_SIZE;
    sprintf(msg.data, "%ld", structStat.st_size);
    msg.len = strlen(msg.data) + 1;
    send_msg(socketFd, &msg);
    
    // 发送完msg后等待回复
    recv_msg(socketFd, &msg);
    if (msg.type == FAILURE) {
        perror("msg failure\n");
        return -1;
    }

    printf("sending file data: %s...\n", filename);
    int i = 0;
    while ((read_length = read(fd, buf, sizeof(buf))) != 0) {
        if (read_length == -1) {
            perror("read error\n");
            return -1;
        }
        msg.type = FILE_DATA;
        msg.len = read_length;
        msg.data = buf;
        send_msg(socketFd, &msg);
        sent_length += read_length;
        for (; i<((float)sent_length / structStat.st_size * 50); i++) {
            putchar('#');
            fflush(stdout);
        }
    }
    close(fd);
    // 文件发送结束
    msg.type = FILE_OVER;
    send_msg(socketFd, &msg);
    // 确认对方是否已收到文件发送结束的消息
    recv_msg(socketFd, &msg);
    if (msg.type == SUCCESS) {
        printf("\n%s file has been sent successfully(has been sent %ld/%ld bytes)\n", filename, atol(msg.data), structStat.st_size);
    } else {
        printf("\n%s file has been sent unsuccessfully(has been sent %ld/%ld bytes)\n", filename, atol(msg.data), structStat.st_size);
    }
    return 0;
}

int recv_file(int socketFd, char* newname) {
    struct ftpmsg msg;
    char* filename;
    mode_t mode; // 文件模式
    long size; // 文件大小
    int fd;
    int write_length = 0;
    long recv_length = 0;

    recv_msg(socketFd, &msg);
    filename = msg.data;

    recv_msg(socketFd, &msg);
    mode = atoi(msg.data);

    recv_msg(socketFd, &msg);
    size = atol(msg.data);
    if (newname != NULL && strcmp(newname, "") != 0) {
        filename = newname;
    }
    // 创建文件
    fd = open(filename, O_WRONLY | O_TRUNC | O_CREAT, mode % 01000);
    if (fd == -1) {
        // 文件创建失败
        perror("open file error\n");
        msg.type = FAILURE;
        send_msg(socketFd, &msg);
        return -1;
    }
    // 文件创建成功
    msg.type = SUCCESS;
    send_msg(socketFd, &msg);

    // 接收文件数据
    printf("receiving file data: %s...\n", filename);
    int i = 0;
    recv_msg(socketFd, &msg);
    while(msg.type != FILE_OVER) {
        if ((write_length = write(fd, msg.data, msg.len)) == -1) {
            perror("write file error\n");
            printf("\nreceiving file %s failure(has received %ld/%ld bytes)\n", filename, recv_length, size);
            msg.type = FAILURE;
            msg.len = sizeof(long);
            sprintf(msg.data, "%ld", recv_length);
            send_msg(socketFd, &msg);
            close(fd);
            return -1;
        }
        recv_length += write_length;
        for (; i < ((double)recv_length / size * 50); i++) {
            putchar('#');
            fflush(stdout);
        }
        recv_msg(socketFd, &msg);
    }
    msg.type = SUCCESS;
    msg.data = malloc(sizeof(long));
    sprintf(msg.data, "%ld", recv_length);
    msg.len = strlen(msg.data);
    send_msg(socketFd, &msg);
    close(fd);
    printf("\n%s has received successfully(has been received %ld/%ld bytes)\n", filename, recv_length, size);
    return 0;
}