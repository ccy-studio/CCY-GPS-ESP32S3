#include "custom_font.h"
#include "math.h"
#include "store.h"
#include "ui.h"

static ui_data_t* _this;            // 自身对象
static lv_obj_t* page_view = NULL;  // 主Root根页面对象
static lv_obj_t* last_focus;        // 标记-记录上一次的焦点对象

static void* bus_gps_info; /*GPS数据*/

/* 组件定义 */
static lv_obj_t* text_title = NULL;  // 顶部标题栏文字对象
static lv_obj_t* menu = NULL;        // Menu定义对象
static lv_obj_t* menu_main = NULL;   // 主菜单容器
/* 子菜单页面定义 */
static lv_obj_t* sub_page_dail;     // 子页面-表盘样式
static lv_obj_t* sub_page_sleep;    // 子页面-自动休眠
static lv_obj_t* sub_page_run_log;  // 子页面-骑行记录
static lv_obj_t* sub_page_gps_pos;  // 子页面-GPS定位信息
static lv_obj_t* sub_page_about;    // 子页面-关于本机

static void on_menu_item_click_envent(lv_event_t* e);
static lv_obj_t* create_menu_item(const char* text, const char* icon_path);
static void on_menu_item_selected_envent(lv_event_t* e);
static void create_sub_page();
static lv_obj_t* create_select_item(const char* text, lv_obj_t* cont);
static void set_focus(lv_obj_t* obj);
static lv_obj_t* create_run_log_item(app_run_log_t* log);
static void create_about_page();
static void on_power_close(lv_event_t* e);
static void on_exit(lv_event_t* e);

/*  设置选项  */
static const char* app_conf_dail[2] = {"科技黑", "简约白"};  // 表盘的样式设置
static const char* app_conf_sleep[4] = {"关闭", "60秒", "1小时",
                                        "2小时"};  // 自动休眠时间的设置

/**
 * @brief 返回PageView对象的指针
 * @return
 */
static lv_obj_t* fun_get_view() {
    return page_view;
}

/**
 * @brief 页面创建时执行
 * @param params
 */
static void on_create_fun(ui_data_t* ui_dat, void* params) {
    _this = ui_dat;
    page_view = lv_obj_create(NULL);
    lv_group_add_obj(ui_dat->group, page_view);
    lv_obj_clear_flag(page_view, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_state(page_view, LV_STATE_DISABLED);

    lv_obj_set_style_bg_color(page_view, lv_color_hex(0xF5F5F5),
                              LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(page_view, 255, LV_PART_MAIN | LV_STATE_DEFAULT);

    lv_obj_t* ui_Image2 = lv_img_create(page_view);
    lv_img_set_src(ui_Image2, "S:/img/setting.bin");
    lv_obj_set_width(ui_Image2, LV_SIZE_CONTENT);
    lv_obj_set_height(ui_Image2, LV_SIZE_CONTENT);
    lv_obj_set_x(ui_Image2, 20);
    lv_obj_set_y(ui_Image2, 10);
    lv_obj_add_flag(ui_Image2, LV_OBJ_FLAG_ADV_HITTEST);
    lv_obj_clear_flag(ui_Image2, LV_OBJ_FLAG_SCROLLABLE);

    text_title = lv_label_create(page_view);
    lv_obj_set_width(text_title, LV_SIZE_CONTENT);
    lv_obj_set_height(text_title, LV_SIZE_CONTENT);
    lv_obj_set_x(text_title, 50);
    lv_obj_set_y(text_title, 10);
    lv_label_set_text(text_title, "设置");
    lv_obj_set_style_text_color(text_title, lv_color_hex(0x000000),
                                LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(text_title, 255, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(text_title, &font_douyin_24,
                               LV_PART_MAIN | LV_STATE_DEFAULT);

    // 创建菜单列表
    menu = lv_menu_create(page_view);
    lv_obj_set_pos(menu, 0, 50);
    lv_obj_set_size(menu, lv_disp_get_hor_res(NULL),
                    lv_disp_get_physical_ver_res(NULL) - 50);
    lv_obj_set_style_bg_color(menu, lv_color_hex(0xF5F5F5), LV_PART_MAIN);
    // 去除Head按钮
    lv_obj_t* head_btn = lv_menu_get_main_header_back_button(menu);
    lv_obj_clean(head_btn);
    lv_obj_remove_style_all(head_btn);
    lv_obj_set_size(head_btn, 0, 0);
    // 创建主菜单选项页面
    menu_main = lv_menu_page_create(menu, NULL);
    lv_obj_set_style_pad_hor(menu_main, 20, 0);
    lv_obj_set_scrollbar_mode(menu_main,
                              LV_SCROLLBAR_MODE_OFF);  // 不显示滚动条
    lv_obj_set_scroll_dir(menu_main, LV_DIR_VER);      // 仅垂直滚动

    create_sub_page();
    lv_obj_t* cont;
    cont = create_menu_item("表盘样式", "S:/img/set_style.bin");
    lv_obj_set_user_data(cont, sub_page_dail);
    lv_menu_set_load_page_event(menu, cont, sub_page_dail);

    cont = create_menu_item("自动休眠", "S:/img/set_sleep.bin");
    lv_obj_set_user_data(cont, sub_page_sleep);
    lv_menu_set_load_page_event(menu, cont, sub_page_sleep);

    cont = create_menu_item("骑行记录", "S:/img/set_log.bin");
    lv_obj_set_user_data(cont, sub_page_run_log);
    lv_menu_set_load_page_event(menu, cont, sub_page_run_log);

    cont = create_menu_item("定位信息", "S:/img/set_gps.bin");
    lv_obj_set_user_data(cont, sub_page_gps_pos);
    lv_menu_set_load_page_event(menu, cont, sub_page_gps_pos);

    cont = create_menu_item("关于本机", "S:/img/set_info.bin");
    lv_obj_set_user_data(cont, sub_page_about);
    lv_menu_set_load_page_event(menu, cont, sub_page_about);

    cont = create_menu_item("退出设置", "S:/img/set_back.bin");
    lv_obj_add_event_cb(cont, on_exit, LV_EVENT_CLICKED, NULL);

    cont = create_menu_item("关机", "S:/img/power.bin");
    lv_obj_add_event_cb(cont, on_power_close, LV_EVENT_CLICKED, NULL);

    lv_menu_set_page(menu, menu_main);
}

/**
 * @brief 创建子页面
 */
static void create_sub_page() {
    lv_obj_t* list;
    lv_obj_t* item;

    sub_page_dail = lv_menu_page_create(menu, NULL);
    list = lv_list_create(sub_page_dail);
    lv_obj_set_size(list, 200, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);  // 不显示滚动条
    lv_obj_set_scroll_dir(list, LV_DIR_VER);                 // 仅垂直滚动
    lv_obj_set_style_radius(list, 10, 0);
    lv_obj_set_style_bg_color(list, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(list, 255, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    // 添加按钮选项
    for (uint8_t i = 0; i < 2; i++) {
        item = create_select_item(app_conf_dail[i], list);
    }

    // 初始自动休眠设置项
    sub_page_sleep = lv_menu_page_create(menu, NULL);
    lv_obj_set_layout(sub_page_sleep, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(sub_page_sleep, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sub_page_sleep, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    list = lv_list_create(sub_page_sleep);
    lv_obj_set_size(list, 200, LV_SIZE_CONTENT);
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);  // 不显示滚动条
    lv_obj_set_scroll_dir(list, LV_DIR_VER);                 // 仅垂直滚动
    lv_obj_set_style_radius(list, 10, 0);
    lv_obj_set_style_bg_color(list, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(list, 255, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_border_width(list, 0, 0);
    // 添加按钮选项
    for (uint8_t i = 0; i < 4; i++) {
        item = create_select_item(app_conf_sleep[i], list);
    }
    item = lv_label_create(sub_page_sleep);
    lv_label_set_text(item, "当未开启骑行时,超时自动关机的时间");
    lv_obj_set_style_text_color(item, lv_color_hex(0x676768), LV_PART_MAIN);
    lv_obj_set_style_text_font(item, &font_douyin_12, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(item, 8, 0);
    lv_obj_set_style_pad_left(item, 10, 0);

    // 初始骑行记录页面
    sub_page_run_log = lv_menu_page_create(menu, NULL);
    lv_obj_set_scrollbar_mode(sub_page_run_log,
                              LV_SCROLLBAR_MODE_OFF);     // 不显示滚动条
    lv_obj_set_scroll_dir(sub_page_run_log, LV_DIR_VER);  // 仅垂直滚动

    // 初始化GPS信息页面
    sub_page_gps_pos = lv_menu_page_create(menu, NULL);
    list = lv_list_create(sub_page_gps_pos);
    lv_obj_set_size(list, lv_pct(100), lv_pct(100));
    lv_obj_set_scrollbar_mode(list, LV_SCROLLBAR_MODE_OFF);  // 不显示滚动条
    lv_obj_set_scroll_dir(list, LV_DIR_VER);                 // 仅垂直滚动
    lv_obj_set_style_radius(list, 0, 0);
    lv_obj_set_style_bg_color(list, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(list, 255, 0);
    lv_obj_set_style_pad_all(list, 0, 0);
    lv_obj_set_style_border_width(list, 0, 0);

    // 初始化关于本机
    sub_page_about = lv_menu_page_create(menu, NULL);
    lv_obj_set_layout(sub_page_about, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(sub_page_about, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(sub_page_about, LV_FLEX_ALIGN_START,
                          LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_START);
    create_about_page();
}

/**
 * @brief 监听点击关机的时间
 * @param e
 */
static void on_power_close(lv_event_t* e) {
    LV_LOG_INFO("Power Close Event...\n");
    ui_fun_finish(_this, true);
}

/**
 * @brief 创建关于本机的页面内容
 * @return
 */
static void create_about_page() {
    lv_obj_t* label;
    util_label_create_pad_b5(sub_page_about, APP_NAME);

    label = util_label_create_pad_b5(sub_page_about, "");
    lv_label_set_text_fmt(label, "版本: %s", APP_VERSION);

    util_label_create_pad_b5(sub_page_about, "官网: saisaiwa.com");
    util_label_create_pad_b5(sub_page_about, "合作联系: yustart@foxmail.com");
    util_label_create_pad_b5(sub_page_about, "使用手册: doc.saisaiwa.com");
}

/** 更新显示GPS信息内容
 * @brief
 * @return
 */
static void update_gps_info_message(app_gps_t* gps) {
    static const char* ANT_OK = "良好";
    static const char* ANT_FIL = "未连接";
    static lv_obj_t *lat = NULL, *lon, *alt, *curr_speed, *sum_mileage,
                    *sum_run_time, *ant_state, *link_satellite_count;
    if (!lv_obj_is_valid(lat)) {
        lv_obj_t* list = lv_obj_get_child(sub_page_gps_pos, 0);
        lon = lv_list_add_btn(list, NULL, "-");
        lat = lv_list_add_btn(list, NULL, "-");
        alt = lv_list_add_btn(list, NULL, "-");
        curr_speed = lv_list_add_btn(list, NULL, "-");
        sum_mileage = lv_list_add_btn(list, NULL, "累计里程:159Km");
        sum_run_time = lv_list_add_btn(list, NULL, "累计骑行时长:153小时");
        ant_state = lv_list_add_btn(list, NULL, "-");
        link_satellite_count = lv_list_add_btn(list, NULL, "-");
        // 统一修改样式
        uint8_t cnt = lv_obj_get_child_cnt(list);
        for (uint8_t i = 0; i < cnt; i++) {
            lv_obj_t* label = lv_obj_get_child(list, i);
            lv_obj_set_style_bg_color(label, lv_color_white(), 0);
            lv_obj_set_style_pad_ver(label, 5, 0);
            lv_obj_set_style_pad_hor(label, 10, 0);
        }
    }
    int speed_kmh = roundf((gps->speed * 3.6));
    lv_label_set_text_fmt(lv_obj_get_child(lon, 0), "经度:%f", gps->longitude);
    lv_label_set_text_fmt(lv_obj_get_child(lat, 0), "纬度:%f", gps->latitude);
    lv_label_set_text_fmt(lv_obj_get_child(alt, 0), "海拔:%f", gps->altitude);
    lv_label_set_text_fmt(lv_obj_get_child(curr_speed, 0), "当前速度:%dkm/h",
                          speed_kmh);
    lv_label_set_text_fmt(lv_obj_get_child(ant_state, 0), "天线状态:%s",
                          gps->sats_in_use > 0 ? ANT_OK : ANT_FIL);
    lv_label_set_text_fmt(lv_obj_get_child(link_satellite_count, 0),
                          "已连接卫星:%d个", gps->sats_in_use);
}

/**
 * @brief 创建单条骑行信息Plan面板组件
 * @param log
 * @return
 */
static lv_obj_t* create_run_log_item(app_run_log_t* log) {
    lv_obj_t* label;
    lv_obj_t* plan = lv_obj_create(sub_page_run_log);
    lv_obj_set_size(plan, 230, LV_SIZE_CONTENT);
    lv_obj_align(plan, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_radius(plan, 10, 0);
    lv_obj_set_style_bg_color(plan, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(plan, 255, 0);
    lv_obj_set_style_pad_all(plan, 5, 0);
    lv_obj_set_style_border_width(plan, 0, 0);
    lv_obj_set_layout(plan, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(plan, LV_FLEX_FLOW_COLUMN);
    lv_group_add_obj(_this->group, plan);
    lv_obj_add_event_cb(plan, ui_callback_dynamic_scrollbar, LV_EVENT_FOCUSED,
                        NULL);

    label = lv_obj_create(sub_page_run_log);
    lv_obj_set_size(label, 0, 10);

    label = lv_label_create(plan);
    lv_label_set_text(label, log->date);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_16, LV_PART_MAIN);

    label = lv_label_create(plan);
    lv_label_set_text_fmt(label, "时间:%s~%s", log->start_time, log->end_time);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    label = lv_label_create(plan);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    char buf[50] = {0};
    strcpy(buf, "用时:");
    strcat(buf, format_time(log->run_second));
    lv_label_set_text(label, buf);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    label = lv_label_create(plan);
    lv_label_set_text_fmt(label, "单里程:%ldkm", log->mileage);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    lv_obj_t* row = lv_obj_create(plan);
    lv_obj_set_size(row, lv_pct(100), LV_SIZE_CONTENT);
    lv_obj_set_style_bg_opa(row, 0, 0);
    lv_obj_set_style_pad_all(row, 0, 0);
    lv_obj_set_style_border_width(row, 0, 0);
    lv_obj_set_layout(row, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(row, LV_FLEX_ALIGN_SPACE_BETWEEN,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_SPACE_BETWEEN);
    label = lv_label_create(row);
    lv_label_set_text_fmt(label, "最高速:%dKm/h", log->max_speed);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    label = lv_label_create(row);
    lv_label_set_text_fmt(label, "均速:%dkm/h", log->avg_speed);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    label = lv_label_create(plan);
    lv_label_set_text_fmt(label, "平均温度:%d度", log->avg_temp);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &font_douyin_12, LV_PART_MAIN);

    return plan;
}

/**
 * @brief 设置菜单-子项快速创建
 * @param text
 * @param cont
 * @return
 */
static lv_obj_t* create_select_item(const char* text, lv_obj_t* cont) {
    lv_obj_t* btn;
    lv_obj_t* label;
    btn = lv_obj_create(cont);
    // btn->user_data = text;
    lv_obj_set_user_data(btn, text);
    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_set_size(btn, 200, 30);
    lv_obj_set_style_border_width(btn, 0, 0);
    label = lv_label_create(btn);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &font_douyin_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加选中图标
    lv_obj_t* img = lv_img_create(btn);
    lv_img_set_src(img, "S:/img/set_ok.bin");
    lv_obj_set_height(img, 0);
    lv_obj_align(img, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_add_event_cb(btn, on_menu_item_selected_envent, LV_EVENT_FOCUSED,
                        img);
    lv_obj_add_event_cb(btn, on_menu_item_selected_envent, LV_EVENT_DEFOCUSED,
                        img);

    lv_obj_align(label, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_pad_all(btn, 5, 0);
    lv_obj_set_style_bg_color(btn, lv_color_hex(0xDCDADA), 6);
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_group_add_obj(_this->group, btn);
    return btn;
}

/**
 * @brief 主菜单-Menu子项快速创建
 * @param text
 * @param icon_path
 * @return
 */
static lv_obj_t* create_menu_item(const char* text, const char* icon_path) {
    lv_obj_t* cont;
    lv_obj_t* label;
    lv_obj_t* icon;
    cont = lv_menu_cont_create(menu_main);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_AROUND,
                          LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    icon = lv_img_create(cont);
    lv_img_set_src(icon, icon_path);
    label = lv_label_create(cont);
    lv_obj_set_size(label, LV_SIZE_CONTENT, LV_SIZE_CONTENT);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &font_douyin_16,
                               LV_PART_MAIN | LV_STATE_DEFAULT);
    // 添加Flex占位组件
    lv_obj_t* seat = lv_obj_create(cont);
    lv_obj_set_size(seat, 40, 0);
    // 添加箭头
    lv_obj_t* img = lv_img_create(cont);
    lv_img_set_src(img, "S:/img/set_active.bin");
    lv_obj_set_height(img, 0);

    lv_obj_add_event_cb(cont, on_menu_item_click_envent, LV_EVENT_CLICKED,
                        text);
    lv_obj_add_event_cb(cont, on_menu_item_selected_envent, LV_EVENT_FOCUSED,
                        img);
    lv_obj_add_event_cb(cont, on_menu_item_selected_envent, LV_EVENT_DEFOCUSED,
                        img);
    lv_obj_set_style_radius(cont, 10, LV_STATE_DEFAULT | LV_PART_MAIN);
    lv_obj_set_style_bg_color(cont, lv_color_white(),
                              LV_STATE_DEFAULT | LV_PART_MAIN);
    // lv_obj_set_style_bg_color(cont, lv_color_hex(0xDCDADA), 6);
    // //设置选中的背景颜色
    lv_obj_set_style_bg_color(cont, lv_color_white(), 6);  // 设置选中的背景颜色
    lv_obj_set_style_text_color(label, lv_color_black(), LV_PART_MAIN);
    lv_group_add_obj(_this->group, cont);
    return cont;
}

/**
 * @brief 改变顶部菜单的文字提示、设置此APP设置项的默认值进行回显
 * @param e
 */
static void on_menu_item_click_envent(lv_event_t* e) {
    // 改变顶部状态栏的标题文字内容
    void* use_dat = lv_event_get_user_data(e);
    lv_label_set_text(text_title, ((char*)lv_event_get_user_data(e)));
    lv_obj_t* item = lv_event_get_target_obj(e);
    last_focus = lv_group_get_focused(_this->group);
    // 在这里设置选项的默认值-默认值将会设置为焦点
    if (use_dat == sub_page_dail) {
        lv_obj_t* target = lv_obj_get_child(lv_obj_get_child(sub_page_dail, 0),
                                            app_store_get_conf_dial());
        set_focus(target);
    } else if (use_dat == sub_page_sleep) {
        lv_obj_t* target = lv_obj_get_child(lv_obj_get_child(sub_page_sleep, 0),
                                            app_store_get_conf_sleep());
        set_focus(target);
    } else if (use_dat == sub_page_run_log) {
        lv_obj_clean(sub_page_run_log);
        // 读取保存的数据
        app_run_log_t logs[APP_COUNT_RUN_LOG_SHOW_MAX];
        app_store_read_run_log(logs, APP_COUNT_RUN_LOG_SHOW_MAX);
        for (uint8_t i = 0; i < APP_COUNT_RUN_LOG_SHOW_MAX; i++) {
            if (logs[i].run_second == 0) {
                break;
            }
            create_run_log_item(&logs[i]);
        }
    }
}

/**
 * @brief 被选中时滚动到指定的Y坐标,监听改变并设置新值保存到数据库存储
 * @param e
 */
static void on_menu_item_selected_envent(lv_event_t* e) {
    lv_obj_t* screen = lv_event_get_target(e);
    lv_obj_t* img = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_FOCUSED) {
        lv_obj_set_height(img, 20);
        lv_obj_scroll_to_y(lv_obj_get_parent(screen), lv_obj_get_y(screen),
                           LV_ANIM_ON);
    } else if (code == LV_EVENT_DEFOCUSED) {
        lv_obj_set_height(img, 0);
    }

    // void* use_dat = screen->user_data;
    void* use_dat = lv_obj_get_user_data(screen);
    if (use_dat == NULL) {
        return;
    }
    lv_obj_t* curr_page = lv_menu_get_cur_main_page(menu);
    if (curr_page == sub_page_dail) {
        if (code == LV_EVENT_FOCUSED) {
            // 被选中设置新值
            if (use_dat == app_conf_dail[APP_CONF_DAIL_0]) {
                app_store_save_conf_dial(APP_CONF_DAIL_0);
            } else if (use_dat == app_conf_dail[APP_CONF_DAIL_1]) {
                app_store_save_conf_dial(APP_CONF_DAIL_1);
            } else {
                LV_LOG_INFO("Error===>Illegal options\n");
            }
        }
    } else if (curr_page == sub_page_sleep) {
        if (code == LV_EVENT_FOCUSED) {
            if (use_dat == app_conf_sleep[APP_CONF_SLEEP_0]) {
                app_store_save_conf_sleep(APP_CONF_SLEEP_0);
            } else if (use_dat == app_conf_sleep[APP_CONF_SLEEP_1]) {
                app_store_save_conf_sleep(APP_CONF_SLEEP_1);
            } else if (use_dat == app_conf_sleep[APP_CONF_SLEEP_2]) {
                app_store_save_conf_sleep(APP_CONF_SLEEP_2);
            } else if (use_dat == app_conf_sleep[APP_CONF_SLEEP_3]) {
                app_store_save_conf_sleep(APP_CONF_SLEEP_3);
            } else {
                LV_LOG_INFO("Error===>Illegal options\n");
            }
        }
    }
}

/**
 * @brief 延迟设置此组件获取焦点
 * @param a
 */
static void on_set_focus(lv_anim_t* a) {
    lv_group_focus_obj(a->var);
}
static void set_focus(lv_obj_t* obj) {
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_exec_cb(&a, NULL);
    lv_anim_set_time(&a, 1);
    lv_anim_set_delay(&a, 1);
    lv_anim_set_ready_cb(&a, on_set_focus);
    lv_anim_start(&a);
}

static void on_exit(lv_event_t* e) {
    ui_fun_finish(_this, true);
}

static void on_event_gps_info(void* sub, bus_msg_t* msg) {
    app_gps_t* gps = msg->payload;
    update_gps_info_message(gps);
}

static void on_stop_fun(void* params) {
    // 解除绑定的订阅
    bus_unregister_subscribe(bus_gps_info);
}

static void on_start_fun(void* params) {
    bus_gps_info =
        bus_register_subscribe(DATA_BUS_GPS_REFRESH, on_event_gps_info, NULL);
}

static void on_destoy_fun(void* params) {
    lv_anim_del_all();
}

static void on_index_read_cb(lv_indev_t* indev, lv_indev_data_t* data) {
    app_btn_pck* pck = lv_indev_get_driver_data(indev);
    if (pck->btn_state == APP_BUTTON_PRESS ||
        pck->btn_state == APP_BUTTON_LONG_PRESS) {
        data->state = LV_INDEV_STATE_PRESSED;
    } else {
        data->state = LV_INDEV_STATE_RELEASED;
    }
    if (pck->btn_code == APP_BUTTON_UP) {
        data->key = LV_KEY_PREV;
    } else if (pck->btn_code == APP_BUTTON_DOWN) {
        data->key = LV_KEY_NEXT;
    } else if (pck->btn_code == APP_BUTTON_ENTER) {
        data->key = LV_KEY_ENTER;
    }

    // 如果是子菜单就退出到主菜单
    if (data->key == LV_KEY_ENTER &&
        lv_menu_get_cur_main_page(menu) != menu_main) {
        // 子菜单页内就退出到主菜单页
        lv_obj_send_event(lv_menu_get_main_header_back_button(menu),
                          LV_EVENT_CLICKED, NULL);
        // 还原之前的标题内容和焦点对象
        lv_label_set_text(text_title, "设置");
        if (last_focus != NULL) {
            lv_group_focus_obj(last_focus);
        }
    }
}

ui_data_t page_setting = {.id = ACTIVITY_ID_SETTING,
                          .launcher_mode = SINGLE_TOP,
                          .fun_get_view = fun_get_view,
                          .fun_on_create = on_create_fun,
                          .fun_on_destoy = on_destoy_fun,
                          .fun_on_start = on_start_fun,
                          .fun_read_cb = on_index_read_cb,
                          .fun_on_stop = on_stop_fun};
