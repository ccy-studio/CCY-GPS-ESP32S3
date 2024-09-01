#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mergin_fsys.h"

#ifndef BIN_DIR
#define BIN_DIR ""
#endif

struct mergin_bin_t bin_infos[] = {
    {.item_name = "dark-dial.bin", .start_address = 0, .end_address = 115204},
    {.item_name = "icon-charge.bin",
     .start_address = 115204,
     .end_address = 115856},
    {.item_name = "icon-gps.bin",
     .start_address = 115856,
     .end_address = 116628},
    {.item_name = "icon-record-start.bin",
     .start_address = 116628,
     .end_address = 118507},
    {.item_name = "icon-record-stop.bin",
     .start_address = 118507,
     .end_address = 120386},
    {.item_name = "icon-s-about.bin",
     .start_address = 120386,
     .end_address = 121590},
    {.item_name = "icon-s-dial.bin",
     .start_address = 121590,
     .end_address = 122794},
    {.item_name = "icon-s-gps.bin",
     .start_address = 122794,
     .end_address = 123998},
    {.item_name = "icon-s-ok.bin",
     .start_address = 123998,
     .end_address = 125202},
    {.item_name = "icon-s-power.bin",
     .start_address = 125202,
     .end_address = 126406},
    {.item_name = "icon-s-right.bin",
     .start_address = 126406,
     .end_address = 127610},
    {.item_name = "icon-s-run-log.bin",
     .start_address = 127610,
     .end_address = 128814},
    {.item_name = "icon-s-sleep.bin",
     .start_address = 128814,
     .end_address = 130018},
    {.item_name = "icon_setting.bin",
     .start_address = 130018,
     .end_address = 131750},
};
size_t bin_infos_len = 14;

void* bin_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode) {
    struct mergin_fs_t* fs = lv_malloc(sizeof(struct mergin_fs_t));
    if (fs == NULL) {
        return NULL;
    }
    fs->mode = mode;
    fs->path = path;
    fs->mergin_bin = find_bin_by_name(strrchr(path, '/') + 1);
    fs->user_data = fopen(BIN_DIR, "rb");
    if (fs->user_data == NULL) {
        LV_LOG_ERROR("找不到/merge.bin文件打开失败");
        lv_free(fs);
        return NULL;
    }
    fseek(fs->user_data, fs->mergin_bin->start_address, SEEK_SET);
    return fs;
}

lv_fs_res_t bin_close_cb(lv_fs_drv_t* drv, void* file_p) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    if (fs->user_data != NULL) {
        fclose(fs->user_data);
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
    *br = fread(buf, 1, btr, fs->user_data);
    return (*br > 0) ? LV_FS_RES_OK : LV_FS_RES_UNKNOWN;  // 返回结果
}

lv_fs_res_t bin_seek_cb(lv_fs_drv_t* drv,
                        void* file_p,
                        uint32_t pos,
                        lv_fs_whence_t whence) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    if (whence == LV_FS_SEEK_SET) {
        return fseek(fs->user_data, fs->mergin_bin->start_address + pos,
                     SEEK_SET) != 0
                   ? LV_FS_RES_UNKNOWN
                   : LV_FS_RES_OK;
    } else if (whence == LV_FS_SEEK_END) {
        return fseek(fs->user_data, fs->mergin_bin->end_address + (pos),
                     SEEK_SET) != 0
                   ? LV_FS_RES_UNKNOWN
                   : LV_FS_RES_OK;
    } else if (whence == LV_FS_SEEK_CUR) {
        return fseek(fs->user_data, pos, SEEK_CUR) != 0 ? LV_FS_RES_UNKNOWN
                                                        : LV_FS_RES_OK;
    }
    return LV_FS_RES_UNKNOWN;
}

lv_fs_res_t bin_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p) {
    struct mergin_fs_t* fs = (struct mergin_fs_t*)file_p;
    long pos = ftell(fs->user_data);
    *pos_p = pos - fs->mergin_bin->start_address;
    return *pos_p < fs->mergin_bin->end_address ? LV_FS_RES_OK
                                                : LV_FS_RES_UNKNOWN;
}