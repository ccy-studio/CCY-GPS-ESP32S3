
/**
 * @file main
 *
 */

/*********************
 *      INCLUDES
 *********************/
#define _DEFAULT_SOURCE /* needed for usleep() */
// #include <lv_demos.h>
// #include <lv_examples.h>
#include <lvgl.h>
#include <pthread.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include "SDL.h"
#include "ui.h"

/*********************
 *      DEFINES
 *********************/
#define MY_DISP_HOR_RES 240
#define MY_DISP_VER_RES 280
/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_display_t* hal_init(int32_t w, int32_t h);
static void random_start();

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

/*********************
 *      DEFINES
 *********************/

extern lv_indev_t* create_custom_indev();

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      VARIABLES
 **********************/
app_environment_t global_env;
app_gps_t global_gps;
app_battery_t global_battery;
/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

int main(int argc, char** argv) {
    (void)argc; /*Unused*/
    (void)argv; /*Unused*/

    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        SDL_Log("SDL 无法初始化! SDL_Error: %s\n", SDL_GetError());
        return -1;
    }

    SDL_Log("启动成功!");

    /*Initialize LVGL*/
    lv_init();

    /*Initialize the HAL (display, input devices, tick) for LVGL*/
    hal_init(MY_DISP_HOR_RES, MY_DISP_VER_RES);

    lv_log("lvgl 日志配置测试");

    ui_init_and_start();

    random_start();

    // lv_demo_widgets();

    while (1) {
        /* Periodically call the lv_task handler.
         * It could be done in a timer interrupt or an OS task too.*/
        lv_timer_handler();
        usleep(5 * 1000);
    }

    return 0;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void sdl_log_cb(lv_log_level_t level, const char* buf) {
    SDL_LogInfo(0, buf);
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t* hal_init(int32_t w, int32_t h) {
    lv_group_set_default(lv_group_create());

    lv_display_t* disp = lv_sdl_window_create(w, h);

    lv_display_set_default(disp);
    lv_indev_t* kb = create_custom_indev();
    lv_indev_set_group(kb, lv_group_get_default());
    lv_indev_set_display(kb, disp);

    // 日志打印的注册
    lv_log_register_print_cb(sdl_log_cb);

    return disp;
}

static void* thread_task(void* args) {
    sleep(5);
    unsigned int seed = time(NULL);  // 使用当前时间作为种子
    global_real_record.is_start = true;
    global_real_record.curr_log_dat.run_second = 0;
    app_start_record();
    while (1) {
        // 填充电池信息
        global_battery.level =
            (float)(rand_r(&seed) % 101);  // 随机电量百分比 0-100%
        global_battery.is_charge = rand_r(&seed) % 2;  // 随机是否在充电

        // 填充环境数据
        global_env.temp = (float)(rand_r(&seed) % 41);  // 随机温度 0-40°C
        global_env.humidity = (float)(rand_r(&seed) % 101);  // 随机湿度 0-100%

        // 填充GPS数据
        global_gps.latitude =
            (double)(rand_r(&seed) % 180) - 90;  // 随机纬度 -90到90
        global_gps.longitude =
            (double)(rand_r(&seed) % 360) - 180;  // 随机经度 -180到180
        global_gps.altitude =
            (double)(rand_r(&seed) % 5000);  // 随机海拔 0-5000米
        global_gps.speed = (double)(rand_r(&seed) % 27);  // 随机速度 0-100 m/s
        global_gps.sats_in_use =
            (uint8_t)(rand_r(&seed) % 12);  // 随机卫星数量 0-12
        time_t now = time(NULL);
        struct tm* local = localtime(&now);
        global_gps.datetime.hour = local->tm_hour;
        global_gps.datetime.minute = local->tm_min;

        // 填充骑行日志数据
        snprintf(global_real_record.curr_log_dat.date,
                 sizeof(global_real_record.curr_log_dat.date),
                 "2023-10-01");  // 固定日期
        snprintf(global_real_record.curr_log_dat.start_time,
                 sizeof(global_real_record.curr_log_dat.start_time),
                 "08:00:00");  // 固定开始时间
        snprintf(global_real_record.curr_log_dat.end_time,
                 sizeof(global_real_record.curr_log_dat.end_time),
                 "09:00:00");  // 固定结束时间
        global_real_record.curr_log_dat.run_second++;

        memcpy(&global_real_record.curr_gps, &global_gps, sizeof(app_gps_t));
        notify_battery_event(&global_battery);
        notify_env_refresh(&global_env);
        notify_gps_refresh(&global_gps);
        notify_data_change();
        sleep(1);  // 每秒更新一次
    }
    return NULL;
}

static void random_start() {
    static pthread_t thread_id;
    pthread_create(&thread_id, NULL, thread_task, NULL);
}