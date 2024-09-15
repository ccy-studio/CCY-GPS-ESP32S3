/*
 * @Description:
 * @Blog: saisaiwa.com
 * @Author: ccy
 * @Date: 2024-01-05 16:20:59
 * @LastEditTime: 2024-01-05 16:41:45
 */

#include "store.h"
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "string.h"


#define rand_r(__seed) (__seed == __seed ? rand () : rand ())

void app_store_read_run_log(app_run_log_t* arr, size_t len) {
    memset(arr, 0x00, sizeof(app_run_log_t) * len);
    unsigned int seed = time(NULL);  // 使用当前时间作为种子
    for (uint8_t i = 0; i < len; i++) {
        strcpy(arr[i].start_time, "08:00");
        strcpy(arr[i].end_time, "09:00");
        strcpy(arr[i].date, "2024-01-05");
        arr[i].run_second = seed;
        arr[i].mileage = (rand_r(&seed) % 100);
        arr[i].max_speed = (rand_r(&seed) % 300);
        arr[i].avg_speed = (rand_r(&seed) % 200);
        arr[i].avg_temp = (rand_r(&seed) % 40);
    }
}

/**
 * @brief 写入骑行日志
 * @param arr
 */
void app_store_save_run_log(app_real_record_t* record) {}

/**
 * @brief 清空骑行日志
 */
void app_store_clean_run_log() {}

/**
 * @brief 获取配置-表盘样式
 * @return
 */
uint8_t app_store_get_conf_dial() {
    return APP_CONF_DAIL_1;
}

/**
 * @brief 保存配置-表盘样式
 * @param type
 */
void app_store_save_conf_dial(uint8_t type) {}

/**
 * @brief 获取配置-睡眠配置
 * @return
 */
uint8_t app_store_get_conf_sleep() {
    return APP_CONF_SLEEP_2;
}

/**
 * @brief 保存配置-睡眠设置
 * @param sleep
 */
void app_store_save_conf_sleep(uint8_t sleep) {}
