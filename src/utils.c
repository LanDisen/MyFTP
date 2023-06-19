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

int rm_spaces(char* str) {
    if (str == NULL || strcmp(str, "")) {
        return 0;
    }
    while (str[0] == ' ') {
        strcpy(str, str + 1);
    }
    return 0;
}

int get_token(char* token, char* str) {
    rm_spaces(str);
    if (str == NULL || strcmp(str, "") == 0) {
        return 0;
    }
    int i = 0;
    while (str[i] != '\0' && str[i] != ' ') {
        i += 1;
    }
    if (str[i] == '\0') {
        strcpy(token, str);
        strcpy(str, "\0");
        return 0;
    }
    strncpy(token, str, i);
    token[i] = '\0';
    strcpy(str, str + i + 1);
    rm_spaces(str);
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