#ifndef __GUI_MERGIN
#define __GUI_MERGIN

#ifdef __cplusplus
extern "C" {
#endif
#include <lvgl.h>

struct mergin_bin_t {
    char item_name[30];
    unsigned long start_address;
    unsigned long end_address;
};

struct mergin_fs_t {
    struct mergin_bin_t* mergin_bin;
    lv_fs_mode_t mode;
    const char* path;
    void* user_data;
};

extern struct mergin_bin_t bin_infos[];
extern size_t bin_infos_len;

struct mergin_bin_t* find_bin_by_name(char* name);

void init_mergin_bin_filesystem(void);

void* bin_open_cb(lv_fs_drv_t* drv, const char* path, lv_fs_mode_t mode);

lv_fs_res_t bin_close_cb(lv_fs_drv_t* drv, void* file_p);

lv_fs_res_t bin_read_cb(lv_fs_drv_t* drv,
                        void* file_p,
                        void* buf,
                        uint32_t btr,
                        uint32_t* br);

lv_fs_res_t bin_seek_cb(lv_fs_drv_t* drv,
                        void* file_p,
                        uint32_t pos,
                        lv_fs_whence_t whence);

lv_fs_res_t bin_tell_cb(lv_fs_drv_t* drv, void* file_p, uint32_t* pos_p);

#ifdef __cplusplus
}
#endif
#endif