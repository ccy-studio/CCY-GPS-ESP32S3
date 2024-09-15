#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mergin_fsys.h"

// 获取Flash中 .rodata段内的文件
extern const uint8_t merge_start[] asm("_binary_merge_bin_start");
extern const uint8_t merge_end[] asm("_binary_merge_bin_end");

struct mergin_bin_t bin_infos[] = {
    {.item_name = "battery.bin", .start_address = 0, .end_address = 812},
    {.item_name = "dark_style_bg.bin",
     .start_address = 812,
     .end_address = 135224},
    {.item_name = "fire.bin", .start_address = 135224, .end_address = 136370},
    {.item_name = "gps.bin", .start_address = 136370, .end_address = 136894},
    {.item_name = "power.bin", .start_address = 136894, .end_address = 137706},
    {.item_name = "record_start.bin",
     .start_address = 137706,
     .end_address = 138968},
    {.item_name = "record_stop.bin",
     .start_address = 138968,
     .end_address = 140230},
    {.item_name = "setting.bin",
     .start_address = 140230,
     .end_address = 141394},
    {.item_name = "set_active.bin",
     .start_address = 141394,
     .end_address = 142206},
    {.item_name = "set_back.bin",
     .start_address = 142206,
     .end_address = 143018},
    {.item_name = "set_gps.bin",
     .start_address = 143018,
     .end_address = 143830},
    {.item_name = "set_info.bin",
     .start_address = 143830,
     .end_address = 144642},
    {.item_name = "set_log.bin",
     .start_address = 144642,
     .end_address = 145454},
    {.item_name = "set_ok.bin", .start_address = 145454, .end_address = 146266},
    {.item_name = "set_sleep.bin",
     .start_address = 146266,
     .end_address = 147078},
    {.item_name = "set_style.bin",
     .start_address = 147078,
     .end_address = 147890},
};
size_t bin_infos_len = 16;

void* bin_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    struct mergin_fs_t* fs = lv_malloc(sizeof(struct mergin_fs_t));
    if (fs == NULL) {
        return NULL;
    }
    fs->mode = mode;
    fs->path = path;
    fs->mergin_bin = find_bin_by_name(strrchr(path, '/') + 1);
    if (fs->mergin_bin == NULL) {
        LV_LOG_ERROR("ERROR: 错误没有找到BIN文件资源");
        return NULL;
    }
    uint32_t* pos = lv_malloc_zeroed(sizeof(uint32_t));
    *pos = 0;
    fs->user_data = pos;
    return fs;
}

lv_fs_res_t bin_close_cb(lv_fs_drv_t* drv, void* file_p) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    if (fs->user_data != NULL) {
        lv_free(fs->user_data);
    }
    lv_free(fs);
    return LV_FS_RES_OK;
}

lv_fs_res_t bin_read_cb(lv_fs_drv_t* drv,
                        void* file_p,
                        void* buf,
                        uint32_t btr,
                        uint32_t* br) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    // 计算数据的起始和结束地址
    const uint8_t* data_start = merge_start + fs->mergin_bin->start_address +
                                *((uint32_t*)fs->user_data);
    const uint8_t* data_end = data_start + btr;
    // 计算要读取的数据长度
    size_t length = data_end - data_start;
    if (length >
        (fs->mergin_bin->end_address - fs->mergin_bin->start_address)) {
        return LV_FS_RES_UNKNOWN;
    }
    // 复制数据
    memcpy(buf, data_start, length);
    *br = btr;
    return (*br > 0) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;  // 返回结果
}

lv_fs_res_t bin_seek_cb(lv_fs_drv_t* drv,
                        void* file_p,
                        uint32_t pos,
                        lv_fs_whence_t whence) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    if (pos > (fs->mergin_bin->end_address - fs->mergin_bin->start_address)) {
        return LV_FS_RES_UNKNOWN;
    }
    if (whence == LV_FS_SEEK_SET) {
        *((uint32_t*)fs->user_data) = pos;
        return LV_FS_RES_OK;
    } else if (whence == LV_FS_SEEK_END) {
        uint32_t diff = fs->mergin_bin->end_address - pos;
        if (diff < fs->mergin_bin->start_address) {
            return LV_FS_RES_UNKNOWN;
        }
        *((uint32_t*)fs->user_data) = diff;
        return LV_FS_RES_OK;
    } else if (whence == LV_FS_SEEK_CUR) {
        if (*((uint32_t*)fs->user_data) + pos > fs->mergin_bin->end_address) {
            return LV_FS_RES_UNKNOWN;
        }
        *((uint32_t*)fs->user_data) += pos;
        return LV_FS_RES_OK;
    }
    return LV_FS_RES_UNKNOWN;
}

lv_fs_res_t bin_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    uint32_t pos = *((uint32_t*)fs->user_data);
    *pos_p = pos;
    return *pos_p < fs->mergin_bin->end_address ? LV_FS_RES_OK
                                                : LV_FS_RES_UNKNOWN;
}