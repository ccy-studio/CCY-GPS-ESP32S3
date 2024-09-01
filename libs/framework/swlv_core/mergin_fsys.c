#include "mergin_fsys.h"
#include <string.h>

struct mergin_bin_t* find_bin_by_name(char* name) {
    if (name == NULL || strlen(name) == 0) {
        return NULL;
    }
    // Iterate through the array to find the matching name
    for (size_t i = 0; i < bin_infos_len; i++) {
        if (strcmp(bin_infos[i].item_name, name) == 0) {
            return &bin_infos[i];  // Return a pointer to the found structure
        }
    }
    return NULL;  // Return NULL if not found
}

void init_mergin_bin_filesystem() {
    static lv_fs_drv_t drv; /*Needs to be static or global*/
    lv_fs_drv_init(&drv);   /*Basic initialization*/

    drv.letter = 'S';            /*An uppercase letter to identify the drive */
    drv.open_cb = bin_open_cb;   /*Callback to open a file */
    drv.close_cb = bin_close_cb; /*Callback to close a file */
    drv.read_cb = bin_read_cb;   /*Callback to read a file */
    drv.seek_cb = bin_seek_cb;   /*Callback to seek in a file (Move cursor) */
    drv.tell_cb = bin_tell_cb;   /*Callback to tell the cursor position  */

    lv_fs_drv_register(&drv); /*Finally register the drive*/
}