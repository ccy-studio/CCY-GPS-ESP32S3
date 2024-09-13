#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "stdio.h"
#include "stdlib.h"
#include "stdint.h"
#include "stdbool.h"

// #define APP_TEST

    /* LVGL-8.3 */

#define APP_VERSION "v1.0.0" /*版本信息*/
#define APP_VERSION_CODE 1 /*版本信息*/
#define APP_NAME "SW Mate" /*产品名称*/

/* 表盘设置 */
#define APP_CONF_DAIL_0 0 /*科技黑*/
#define APP_CONF_DAIL_1 1 /*简约白*/

/* 自动休眠选项设置 */
#define APP_CONF_SLEEP_0 0 /*关闭*/
#define APP_CONF_SLEEP_1 1 /*60秒*/
#define APP_CONF_SLEEP_2 2 /*1小时*/
#define APP_CONF_SLEEP_3 3 /*2小时*/


#define APP_GPS_FRAME 0.5 /*GPS刷新帧率单位秒*/
#define APP_COUNT_RUN_LOG_SHOW_MAX 5 /*最大可查看骑行日志记录的个数*/

/**
 * @brief APP按键物理按键KEY定义
*/
    typedef enum {
        APP_BUTTON_UP = 1,
        APP_BUTTON_DOWN,
        APP_BUTTON_ENTER,
    } app_button_t;

    /**
     * @brief APP物理按键状态定义
    */
    typedef enum {
        APP_BUTTON_RELEASE = 0,
        APP_BUTTON_PRESS,
        APP_BUTTON_LONG_PRESS,
    } app_button_state_t;

    typedef struct {
        app_button_t btn_code;
        app_button_state_t btn_state;
    } app_btn_pck;

    /**
     * @brief 电池电量信息
    */
    typedef struct {
        float level; //剩余电量百分比 0~100%
        bool is_charge; //是否在充电
    } app_battery_t;

    /* 骑行日志-主信息 */
    typedef struct {
        char date[11]; //日期
        char start_time[9]; //开始时间
        char end_time[9]; //结束时间
        uint32_t run_second; //总耗时
        uint32_t mileage; //总里程
        uint16_t max_speed; //最高速
        uint16_t avg_speed; //平均速度
        uint8_t avg_temp; //平均温度
    } app_run_log_t;


    typedef struct {
        uint8_t hour;      /*!< Hour */
        uint8_t minute;    /*!< Minute */
        uint8_t second;    /*!< Second */
        uint16_t thousand; /*!< Thousand */
        uint8_t day;   /*!< Day (start from 1) */
        uint8_t month; /*!< Month (start from 1) */
        uint16_t year; /*!< Year (start from 2000) */
    }_app_gps_datetime_t;

    typedef _app_gps_datetime_t app_gps_dt;

    /*GPS定位信息*/
    typedef struct {
        _app_gps_datetime_t datetime;
        double latitude;                                                /*!< Latitude (degrees) */
        double longitude;                                               /*!< Longitude (degrees) */
        double altitude;                                                /*!< Altitude (meters) */
        double speed;                                                   /*!< Ground speed, unit: m/s */
        uint8_t sats_in_use;                                           /*!< Number of satellites in use */
    }app_gps_t;

    /*传感器环境信息*/
    typedef struct {
        float temp; /*温度*/
        float humidity; /*湿度*/
    }app_environment_t;

    /* 开启骑行后的实时数据记录 */
    typedef struct {
        bool is_start; //是否开启记录
        app_gps_t curr_gps;
        app_run_log_t curr_log_dat;
        app_environment_t env_dat;
        uint32_t tick_environment_cnt;
        uint32_t tick_gps_cnt; //记录平均速度用,计数器
        double speed_sum; //记录当前速度累加和-计算均速用
        double distance; //记录当前行驶的距离单位累加和:m/s
    }app_real_record_t;



/** 全局单例变量 */
extern app_real_record_t global_real_record;
extern app_environment_t global_env;
extern app_gps_t global_gps;
extern app_battery_t global_battery;

#ifdef __cplusplus
}
#endif
