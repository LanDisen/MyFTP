#ifndef _CMD_H
#define _CMD_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>

int input(char* str);

int rm_spaces(char* str);

// 第一个空格前的字符串会传递到token，str删去该部分字符串
int get_token(char* token, char* str);

// 从文件路径中截取文件名
char* get_filename(char* filename, const char* path);

#endif