#ifndef __COMMON_H
#define __COMMON_H

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <fcntl.h>
#include <time.h>
#include <stdbool.h>

// 目录
#include <dirent.h>

// 触摸屏
#include <linux/input.h>

// 液晶屏
#include <sys/mman.h> // 映射内存
#include <linux/fb.h> // LCD设备操作结构体

#endif
