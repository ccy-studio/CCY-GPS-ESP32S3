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
		.item_name = "fire.bin",
		.start_address = 135224,
		.end_address = 136370 
	},
	{ 
		.item_name = "gps.bin",
		.start_address = 136370,
		.end_address = 136894 
	},
	{ 
		.item_name = "power.bin",
		.start_address = 136894,
		.end_address = 137706 
	},
	{ 
		.item_name = "record_start.bin",
		.start_address = 137706,
		.end_address = 138968 
	},
	{ 
		.item_name = "record_stop.bin",
		.start_address = 138968,
		.end_address = 140230 
	},
	{ 
		.item_name = "setting.bin",
		.start_address = 140230,
		.end_address = 141394 
	},
	{ 
		.item_name = "set_active.bin",
		.start_address = 141394,
		.end_address = 142206 
	},
	{ 
		.item_name = "set_back.bin",
		.start_address = 142206,
		.end_address = 143018 
	},
	{ 
		.item_name = "set_gps.bin",
		.start_address = 143018,
		.end_address = 143830 
	},
	{ 
		.item_name = "set_info.bin",
		.start_address = 143830,
		.end_address = 144642 
	},
	{ 
		.item_name = "set_log.bin",
		.start_address = 144642,
		.end_address = 145454 
	},
	{ 
		.item_name = "set_ok.bin",
		.start_address = 145454,
		.end_address = 146266 
	},
	{ 
		.item_name = "set_sleep.bin",
		.start_address = 146266,
		.end_address = 147078 
	},
	{ 
		.item_name = "set_style.bin",
		.start_address = 147078,
		.end_address = 147890 
	},
};
size_t bin_infos_len = 16;