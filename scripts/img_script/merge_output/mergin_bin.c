#include "mergin_bin.h"

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