/*
 * @Description: 
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2024-01-05 16:19:22
 * @LastEditTime: 2024-01-05 16:29:42
 */
#include "util.h"

const char* format_time(uint32_t run_second) {
    int hours, minutes, seconds;
    hours = run_second / 3600;
    minutes = (run_second % 3600) / 60;
    seconds = (run_second % 3600) % 60;
    const char str[50] = { 0 };
    if (hours > 0) {
        sprintf(str, "%d小时%d分钟%d秒", hours, minutes, seconds);
    } else if (minutes > 0) {
        sprintf(str, "%d分钟%d秒", minutes, seconds);
    } else {
        sprintf(str, "%d秒", seconds);
    }
    return str;
}
