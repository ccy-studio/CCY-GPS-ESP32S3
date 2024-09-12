
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

/**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_indev_t* kb;
static lv_indev_read_cb_t red_cb;

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
    kb = create_custom_indev();
    lv_indev_set_group(kb, lv_group_get_default());
    lv_indev_set_display(kb, disp);

    // 日志打印的注册
    lv_log_register_print_cb(sdl_log_cb);

    return disp;
}

lv_indev_t* ui_base_get_indev() {
    return kb;
}