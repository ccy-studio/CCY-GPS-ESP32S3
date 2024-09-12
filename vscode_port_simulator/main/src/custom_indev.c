#include "SDL.h"
#include "app_config.h"
#include "lvgl.h"
#include "ui.h"

extern lv_display_t* lv_sdl_get_disp_from_win_id(uint32_t win_id);

static void sdl_keyboard_read_custom(lv_indev_t* indev, lv_indev_data_t* data) {
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    // lv_log("CB Key=%d,release: %d, PCK: code = %d,state = %d\n", data->key,
    //        data->state == LV_INDEV_STATE_PRESSED, pck->btn_code,
    //        pck->btn_state);

    ui_data_t* curr = ui_get_current();
    if (curr != NULL && curr->fun_read_cb != NULL) {
        curr->fun_read_cb(indev, data);
        return;
    }

    if (pck->btn_state == APP_BUTTON_PRESS ||
        pck->btn_state == APP_BUTTON_LONG_PRESS) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    if (pck->btn_code == APP_BUTTON_UP) {
        data->key = LV_KEY_UP;
    } else if (pck->btn_code == APP_BUTTON_DOWN) {
        data->key = LV_KEY_DOWN;
    } else if (pck->btn_code == APP_BUTTON_ENTER) {
        data->key = LV_KEY_ENTER;
    }
}

static void release_indev_cb(lv_event_t* e) {
    lv_indev_t* indev = (lv_indev_t*)lv_event_get_user_data(e);
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    if (pck) {
        lv_indev_set_driver_data(indev, NULL);
        lv_indev_set_read_cb(indev, NULL);
        lv_free(pck);
        LV_LOG_INFO("done");
    }
}

lv_indev_t* create_custom_indev() {
    lv_indev_t* kb = lv_indev_create();
    app_btn_pck* pck = lv_malloc_zeroed(sizeof(app_btn_pck));
    LV_ASSERT_MALLOC(pck);
    if (pck == NULL) {
        return NULL;
    }
    lv_indev_set_driver_data(kb, pck);
    lv_indev_set_type(kb, LV_INDEV_TYPE_KEYPAD);
    lv_indev_set_mode(kb, LV_INDEV_MODE_EVENT);
    lv_indev_set_read_cb(kb, sdl_keyboard_read_custom);
    lv_indev_add_event_cb(kb, release_indev_cb, LV_EVENT_DELETE, kb);
    return kb;
}

void lv_sdl_keyboard_handler(SDL_Event* event) {
    uint32_t win_id = UINT32_MAX;
    switch (event->type) {
        case SDL_KEYDOWN:
            win_id = event->key.windowID;
            break;
        case SDL_TEXTINPUT:
            win_id = event->text.windowID;
            break;
        default:
            return;
    }

    lv_display_t* disp = lv_sdl_get_disp_from_win_id(win_id);

    /*Find a suitable indev*/
    lv_indev_t* indev = lv_indev_get_next(NULL);
    while (indev) {
        if (lv_indev_get_type(indev) == LV_INDEV_TYPE_KEYPAD) {
            /*If disp is NULL for any reason use the first indev with the
             * correct type*/
            if (disp == NULL || lv_indev_get_display(indev) == disp)
                break;
        }
        indev = lv_indev_get_next(indev);
    }
    if (indev == NULL)
        return;

    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    /* We only care about SDL_KEYDOWN and SDL_TEXTINPUT events */
    switch (event->type) {
        case SDL_KEYDOWN: { /*Button press*/
            // const uint32_t ctrl_key =
            // keycode_to_ctrl_key(event->key.keysym.sym); if(ctrl_key == '\0')
            //     return;
            // lv_log("SDL Key =%ld\n", event->key.keysym.sym);
            switch (event->key.keysym.sym) {
                case 1073741906:
                case 119:
                    pck->btn_code = APP_BUTTON_UP;
                    break;
                case 1073741905:
                case 115:
                    pck->btn_code = APP_BUTTON_DOWN;
                    break;
                case 13:
                case 120:
                    pck->btn_code = APP_BUTTON_ENTER;
                    break;
            }
            if (event->key.keysym.sym == 27) {
                pck->btn_state = APP_BUTTON_RELEASE;
            } else if (event->key.keysym.sym > 200 ||
                       event->key.keysym.sym == 13) {
                pck->btn_state = APP_BUTTON_PRESS;
            } else {
                pck->btn_state = APP_BUTTON_LONG_PRESS;
            }
            break;
        }
        case SDL_TEXTINPUT: { /*Text input*/
            lv_log("SDL TextInput = %s\n", event->text.text);
        } break;
        default:
            break;
    }
    lv_indev_read(indev);
    // size_t len = lv_strlen(dsc->buf);
    // while(len) {
    //     lv_indev_read(indev);

    //     /*Call again to handle dummy read in `sdl_keyboard_read`*/
    //     lv_indev_read(indev);
    //     len--;
    // }
}