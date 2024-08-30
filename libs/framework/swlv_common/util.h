#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "app_config.h"

    /**
     * @brief 时间格式化
     * @param run_second 单位秒
     * @return
    */
    const char* format_time(uint32_t run_second);

#ifdef __cplusplus
}
#endif
