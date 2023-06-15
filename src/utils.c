#include "utils.h"

int input(char* str) {
    char ch;
    while ((ch = getchar()) != EOF) {
        if (ch == '\n') break;
        *str++ = ch;
    }
    *str++ = '\0';
    return 0;
}

// int get_token(char* token, char* cmd) {
//     // 去除cmd开头的空格
//     while (*cmd == ' ') {
//         strcpy(cmd, cmd + 1);
//     }
//     int i = 0;
//     while (*cmd != ' ' && *cmd != '\0') {
//         *token++ = *cmd;
//         i++;
//         strcpy(cmd, cmd + 1);
//     }
//     *token++ = '\0';
//     if (i == 0) {
//         return -1;
//     }
//     while (*cmd == ' ') {
//         strcpy(cmd, cmd + 1);
//     }
//     return 0;
// }

int get_token(char* token, char* line) {
    int i = 0;
    
    // 清除前导空格
    while (isspace(line[i])) {
        i++;
    }

    if (strcmp(line, "") == 0) {
        return -1;
    }

    if (strchr(line, ' ') == NULL) {
        strcpy(token, line);
        strcpy(line, "");
        return 0;
    }
    
    // 复制单词到token
    int j = 0;
    while (line[i] != '\0' && !isspace(line[i])) {
        token[j] = line[i];
        i++;
        j++;
    }
    token[j] = '\0';
    
    // 截取line中第一个空格之后的内容
    if (line[i] != '\0') {
        i++;  // 跳过第一个空格
        int k = 0;
        while (line[i] != '\0') {
            line[k] = line[i];
            i++;
            k++;
        }
        line[k] = '\0';
    }
    
    return 0;
}

char* get_filename(char* filename, const char* path) {
    if (strchr(path, '/') == NULL) {
        strcpy(filename, path);
    } else {
        strcpy(filename, strrchr(path, '/') + 1);
    }
    return filename;
}