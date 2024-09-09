#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mergin_fsys.h"

#ifndef BIN_DIR
#define BIN_DIR ""
#endif
struct mergin_bin_t bin_infos[] = {
	{ 
		.item_name = "battery.bin",
		.start_address = 0,
		.end_address = 812 
	},
	{ 
		.item_name = "dark_style_bg.bin",
		.start_address = 812,
		.end_address = 135224 
	},
	{ 
		.item_name = "gps.bin",
		.start_address = 135224,
		.end_address = 135748 
	},
	{ 
		.item_name = "power.bin",
		.start_address = 135748,
		.end_address = 136560 
	},
	{ 
		.item_name = "record_start.bin",
		.start_address = 136560,
		.end_address = 137822 
	},
	{ 
		.item_name = "record_stop.bin",
		.start_address = 137822,
		.end_address = 139084 
	},
	{ 
		.item_name = "setting.bin",
		.start_address = 139084,
		.end_address = 140248 
	},
	{ 
		.item_name = "set_active.bin",
		.start_address = 140248,
		.end_address = 141060 
	},
	{ 
		.item_name = "set_gps.bin",
		.start_address = 141060,
		.end_address = 141872 
	},
	{ 
		.item_name = "set_info.bin",
		.start_address = 141872,
		.end_address = 142684 
	},
	{ 
		.item_name = "set_log.bin",
		.start_address = 142684,
		.end_address = 143496 
	},
	{ 
		.item_name = "set_ok.bin",
		.start_address = 143496,
		.end_address = 144308 
	},
	{ 
		.item_name = "set_sleep.bin",
		.start_address = 144308,
		.end_address = 145120 
	},
	{ 
		.item_name = "set_style.bin",
		.start_address = 145120,
		.end_address = 145932 
	},
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
    if (fs->mergin_bin == NULL) {
        return NULL;
    }
    fs->user_data = fopen(BIN_DIR, "rb");
    if (fs->user_data == NULL) {
        lv_log("找不到/merge.bin文件打开失败");
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
        return fseek(fs->user_data, fs->mergin_bin->end_address - pos,
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