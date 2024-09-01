#include "ui.h"

#define MAX_STACK_COUNT 10  // 最大栈的数量
#define _ANIM_DELAY 200

/**
 * 双向链表结构体
 */
typedef struct activity_stack_link {
    ui_data_t* ui_data;
    struct activity_stack_link* next;
    struct activity_stack_link* pre;
} activity_stack_link_t;

static activity_stack_link_t l_root;       // 定义链表
static activity_stack_link_t* l_bottom_p;  // 栈顶顶层，当前活动的Activity
static uint8_t ui_stack_index = 1;         // 标记当前栈内元素数量

static lv_obj_t* _init_empty_scr;

extern ui_data_t* ui_data_def[MAX_PAGE];
extern void register_ui_def();
static void ui_deinit_group(ui_data_t* ui_dat);

// KeyDev
// extern lv_indev_t* lv_win32_keypad_device_object;

void ui_init_and_start() {
    // 这里去注册activity
    register_ui_def();
    lv_disp_t* dispp = lv_disp_get_default();
    lv_theme_t* theme = lv_theme_default_init(
        dispp, lv_palette_main(LV_PALETTE_BLUE),
        lv_palette_main(LV_PALETTE_RED), false, LV_FONT_DEFAULT);
    lv_disp_set_theme(dispp, theme);
    // 寻找启动页并初始化
    for (int i = 0; i < MAX_PAGE; i++) {
        // 如果是Launcher启动页就初始化为root根节点的链表头
        if (ui_data_def[i]->launcher) {
            l_root.ui_data = ui_data_def[i];
            l_root.next = NULL;
            l_root.pre = NULL;
            l_bottom_p = &l_root;
            if (l_root.ui_data->fun_on_create != NULL) {
                l_root.ui_data->fun_on_create(l_root.ui_data, NULL);
                l_root.ui_data->is_createed = true;
            }
            if (l_root.ui_data->fun_on_start != NULL) {
                l_root.ui_data->fun_on_start(NULL);
            }
            _ui_load_scr(l_root.ui_data->fun_get_view());
            break;
        }
    }
    _init_empty_scr = lv_obj_create(NULL);
}

static ui_data_t* find_by_id(activity_id_t id) {
    for (int i = 0; i < MAX_PAGE; i++) {
        if (ui_data_def[i]->id == id) {
            return ui_data_def[i];
        }
    }
    return NULL;
}

/**
 * 栈顶压栈
 */
static void push_stack(ui_data_t* activity) {
    if (activity == NULL) {
        return;
    }
    if (ui_stack_index >= MAX_STACK_COUNT) {
        ui_error("fail,The number of stack tops exceeds the maximum value");
        return;
    }
    if (activity->launcher_mode == SINGLE_TOP) {
        if (l_bottom_p->ui_data == activity) {
            return;
        }
    } else if (activity->launcher_mode == SINGLE_TASK) {
        if (l_bottom_p->ui_data == activity) {
            return;
        } else {
            activity_stack_link_t* p = &l_root;
            bool del = false;
            ui_stack_index = 0;
            while (p != NULL) {
                ui_stack_index++;
                if (p->ui_data == activity) {
                    del = true;
                    break;
                }
                p = p->next;
            }
            if (del) {
                activity_stack_link_t* p1 = p->next;
                while (p1 != NULL) {
                    if (p1->ui_data->fun_on_stop != NULL) {
                        p1->ui_data->fun_on_stop(NULL);
                    }
                    if (p1->ui_data->fun_on_destoy != NULL) {
                        p1->ui_data->fun_on_destoy(NULL);
                    }
                    ui_deinit_group(p1->ui_data);
                    activity_stack_link_t* temp = p1->next;
                    lv_obj_del(p1->ui_data->fun_get_view());
                    free(p1);
                    p1 = temp;
                }
                l_bottom_p = p;
                return;
            }
        }
    }
    activity_stack_link_t* p =
        (activity_stack_link_t*)malloc(sizeof(activity_stack_link_t));
    if (p == NULL) {
        ui_error(
            "Error: Failed to create memory space while pressing the stack");
        return;
    }
    p->ui_data = activity;
    p->next = NULL;
    p->pre = l_bottom_p;
    l_bottom_p->next = p;
    l_bottom_p = p;
    ui_stack_index++;
}

/**
 * 出栈
 */
static void pull_stack(ui_data_t* activity) {
    if (activity == l_root.ui_data) {
        activity_stack_link_t* next = l_root.next;
        l_root.ui_data = next->ui_data;
        l_root.next = next->next;
        l_root.pre = NULL;
        ui_stack_index--;
        if (l_bottom_p == next) {
            l_bottom_p = &l_root;
        }
        free(next);
        return;
    }
    activity_stack_link_t* p = l_root.next;
    while (p != NULL) {
        if (p->ui_data == activity) {
            // 将上一级指向的Next指针指向自身的Next指针
            p->pre->next = p->next;
            if (p == l_bottom_p) {
                l_bottom_p = p->pre;
            }
            free(p);
            ui_stack_index--;
            break;
        }
        p = p->next;
    }
}

/**
 * 创建Intent意图
 */
void ui_fun_fast_create_intent(ui_data_t* _this,
                               activity_id_t target,
                               ui_intent_t* intent) {
    ui_data_t* target_data = find_by_id(target);
    if (_this == NULL || target_data == NULL) {
        ui_error("fail,Unable to find data based on ID(ui_fun_fast_create)");
        return;
    }
    intent->_this = _this;
    intent->target = target_data;
}

/**
 * 跳转Activity
 */
void ui_fun_start_activity(ui_intent_t* intent) {
    if (intent->_this->fun_on_stop != NULL) {
        intent->_this->fun_on_stop(NULL);
    }
    if (intent->target->fun_on_create != NULL && !intent->target->is_createed) {
        intent->target->fun_on_create(intent->target, NULL);
        intent->target->is_createed = true;
    }
    if (intent->target->fun_on_start != NULL) {
        intent->target->fun_on_start(NULL);
    }
    if (intent->anim) {
        _ui_load_scr_opa(intent->target->fun_get_view());
    } else {
        _ui_load_scr(intent->target->fun_get_view());
    }
    push_stack(intent->target);
}

static void on_activity_unload_del(lv_event_t* e) {
    // ui_data_t* this_data = (ui_data_t*)e->user_data;
    ui_data_t* this_data = lv_event_get_user_data(e);
    lv_obj_del(lv_event_get_target(e));
    this_data->is_createed = false;
}

/**
 * 关闭并销毁当前的Activity
 */
void ui_fun_finish(ui_data_t* _this, bool anim) {
    if (_this == NULL) {
        ui_error("fail,Unable to find data based on ID(ui_fun_finish)");
        return;
    }
    if (ui_stack_index == 1) {
        ui_error("fail Keep at least one activity");
        return;
    }
    if (_this->fun_on_stop != NULL) {
        _this->fun_on_stop(NULL);
    }
    if (_this->fun_on_destoy != NULL) {
        _this->fun_on_destoy(NULL);
    }
    ui_deinit_group(_this);
    bool is_top = l_bottom_p->ui_data == _this;
    pull_stack(_this);
    if (is_top) {
        // 如果被出栈的是栈顶那么需要激活显示后一级的页面
        if (l_bottom_p->ui_data->fun_on_restart != NULL) {
            l_bottom_p->ui_data->fun_on_restart(NULL);
        }
        if (l_bottom_p->ui_data->fun_on_start != NULL) {
            l_bottom_p->ui_data->fun_on_start(NULL);
        }
        if (anim) {
            _ui_load_scr_opa(l_bottom_p->ui_data->fun_get_view());
        } else {
            _ui_load_scr(l_bottom_p->ui_data->fun_get_view());
        }
    }
    lv_obj_add_event_cb(_this->fun_get_view(), on_activity_unload_del,
                        LV_EVENT_SCREEN_UNLOADED, _this);
}

void _ui_load_scr(lv_obj_t* scr) {
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_NONE, 0, 0, false);
}
void _ui_load_scr_opa(lv_obj_t* scr) {
    lv_scr_load_anim(scr, LV_SCR_LOAD_ANIM_OVER_LEFT, _ANIM_DELAY, 0, false);
}

/**
 * @brief 向栈顶当前活动的Activity发送Event事件
 * @param event 事件类型
 * @param params 用户数据
 */
void ui_send_event_current(bus_event event, void* params) {
    if (l_bottom_p->ui_data->fun_handle_event != NULL) {
        l_bottom_p->ui_data->fun_handle_event(event, params);
    }
}

void ui_send_event(activity_id_t activity, bus_event event, void* params) {
    ui_data_t* ui_dat = find_by_id(activity);
    if (ui_dat == NULL) {
        ui_error("fail,Unable to find data based on ID(ui_fun_finish)");
        return;
    }
    if (ui_dat->fun_handle_event != NULL) {
        ui_dat->fun_handle_event(event, params);
    }
}

void ui_send_event_all(bus_event event, void* params) {
    activity_stack_link_t* root = l_root.next;
    while (root != NULL) {
        if (root->ui_data->fun_handle_event != NULL) {
            root->ui_data->fun_handle_event(event, params);
        }
        root = l_root.next;
    }
}

void ui_init_group(ui_data_t* ui_dat) {
    if (ui_dat->group == NULL) {
        ui_dat->group = lv_group_create();
        lv_group_set_default(ui_dat->group);
        // lv_indev_set_group(ui_base_get_indev(), ui_dat->group);
        /*     lv_group_set_editing(ui_dat->group, false);*/
    }
}

/**
 * @brief 释放销毁GroupKey
 * @param ui_dat
 */
static void ui_deinit_group(ui_data_t* ui_dat) {
    if (ui_dat->group != NULL) {
        lv_group_remove_all_objs(ui_dat->group);
        // lv_group_del(ui_dat->group);
    }
}

void ui_callback_dynamic_scrollbar(lv_event_t* e) {
    lv_obj_t* obj = lv_event_get_target(e);
    lv_obj_scroll_to_y(lv_obj_get_parent(obj), lv_obj_get_y(obj), LV_ANIM_ON);
}

void ui_error(char* error) {
    LV_LOG_INFO("(GUI Activity)ERROR==>%s\n", error);
}
