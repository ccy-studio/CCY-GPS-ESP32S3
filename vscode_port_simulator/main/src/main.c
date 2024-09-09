
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

static void sdl_keyboard_read_custom(lv_indev_t* indev, lv_indev_data_t* data) {
    red_cb(indev, data);
    lv_log("Key=%d,release: %d\n", data->key,
           data->state == LV_INDEV_STATE_PRESSED);
    /**
     *  APP_BUTTON_UP = 1,
        APP_BUTTON_DOWN,
        APP_BUTTON_ENTER,
     */
    app_btn_pck pck;
    pck.btn_state = data->key, data->state;

    switch (data->key) {
        case 17:  // APP_BUTTON_UP
            pck.btn_code = APP_BUTTON_UP;
            break;
        case 18:  // APP_BUTTON_DOWN
            pck.btn_code = APP_BUTTON_DOWN;
            break;
        case 10:  // APP_BUTTON_ENTER
            pck.btn_code = APP_BUTTON_ENTER;
            break;

        case 119:  // APP_BUTTON_UP LONG
            pck.btn_code = APP_BUTTON_UP;
            pck.btn_state = APP_BUTTON_LONG_PRESS;
            break;
        case 115:  // APP_BUTTON_DOWN LONG
            pck.btn_code = APP_BUTTON_DOWN;
            pck.btn_state = APP_BUTTON_LONG_PRESS;
            break;
        case 120:  // APP_BUTTON_ENTER LONG
            pck.btn_code = APP_BUTTON_ENTER;
            pck.btn_state = APP_BUTTON_LONG_PRESS;
            break;
        default:
            break;
    }
    notify_button_event(&pck);
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the LVGL graphics
 * library
 */
static lv_display_t* hal_init(int32_t w, int32_t h) {
    lv_group_set_default(lv_group_create());

    lv_display_t* disp = lv_sdl_window_create(w, h);

    // lv_obj_add_event()

    // lv_indev_t* mouse = lv_sdl_mouse_create();
    // lv_indev_set_group(mouse, lv_group_get_default());
    // lv_indev_set_display(mouse, disp);
    lv_display_set_default(disp);

    // LV_IMAGE_DECLARE(mouse_cursor_icon); /*Declare the image file.*/
    // lv_obj_t* cursor_obj;
    // cursor_obj = lv_image_create(
    //     lv_screen_active()); /*Create an image object for the cursor */
    // lv_image_set_src(cursor_obj, &mouse_cursor_icon); /*Set the image
    // source*/ lv_indev_set_cursor(mouse,
    //                     cursor_obj); /*Connect the image  object to the
    //                     driver*/

    // lv_indev_t* mousewheel = lv_sdl_mousewheel_create();
    // lv_indev_set_display(mousewheel, disp);
    // lv_indev_set_group(mousewheel, lv_group_get_default());

    kb = lv_sdl_keyboard_create();
    red_cb = lv_indev_get_read_cb(kb);
    lv_indev_set_read_cb(kb, sdl_keyboard_read_custom);
    lv_indev_set_display(kb, disp);

    lv_indev_set_group(kb, lv_group_get_default());

    // 日志打印的注册
    lv_log_register_print_cb(sdl_log_cb);

    return disp;
}

lv_indev_t* ui_base_get_indev() {
    return kb;
}