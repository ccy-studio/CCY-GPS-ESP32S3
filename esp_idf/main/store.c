#include "store.h"

/**
 * @brief 读取骑行日志
 * @param arr
 * @param len
 */
void app_store_read_run_log(app_run_log_t* arr, size_t len) {}

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
    return APP_CONF_SLEEP_1;
}

/**
 * @brief 保存配置-睡眠设置
 * @param sleep
 */
void app_store_save_conf_sleep(uint8_t sleep) {}