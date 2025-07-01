//Copyright (c) 2022 Lenovo. All right reserved.
//Confidential and Proprietary
#pragma once
#include <windows.h>

void my_log_print(const char* mod, int level, const char* func_name, int line_number, const char* fmt, ...);
void my_record_print(const char* fmt, ...);

#define Print_Debug(fmt, ...) \
    do {my_log_print(MODULE_TAG, 3, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);}while(0)

#define Print_Info(fmt, ...) \
    do {my_log_print(MODULE_TAG, 2, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);}while(0)

#define Print_Error(fmt, ...) \
    do {my_log_print(MODULE_TAG, 1, __FUNCTION__, __LINE__, fmt, ##__VA_ARGS__);}while(0)

#ifdef PUBLISH
#define Print_Record(fmt, ...) do{}while(0)
#else
#define Print_Record(fmt, ...) \
    do {my_record_print(fmt, ##__VA_ARGS__);}while(0)
#endif
