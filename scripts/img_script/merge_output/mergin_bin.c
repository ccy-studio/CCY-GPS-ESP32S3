#include "mergin_bin.h"

struct mergin_bin_t bin_infos[] = {
	{ 
		.item_name = "dark-dial.bin",
		.start_address = 0,
		.end_address = 115212 
	},
	{ 
		.item_name = "icon-charge.bin",
		.start_address = 115212,
		.end_address = 115864 
	},
	{ 
		.item_name = "icon-gps.bin",
		.start_address = 115864,
		.end_address = 116636 
	},
	{ 
		.item_name = "icon-record-start.bin",
		.start_address = 116636,
		.end_address = 118515 
	},
	{ 
		.item_name = "icon-record-stop.bin",
		.start_address = 118515,
		.end_address = 120394 
	},
	{ 
		.item_name = "icon-s-about.bin",
		.start_address = 120394,
		.end_address = 121598 
	},
	{ 
		.item_name = "icon-s-dial.bin",
		.start_address = 121598,
		.end_address = 122802 
	},
	{ 
		.item_name = "icon-s-gps.bin",
		.start_address = 122802,
		.end_address = 124006 
	},
	{ 
		.item_name = "icon-s-ok.bin",
		.start_address = 124006,
		.end_address = 125210 
	},
	{ 
		.item_name = "icon-s-power.bin",
		.start_address = 125210,
		.end_address = 126414 
	},
	{ 
		.item_name = "icon-s-right.bin",
		.start_address = 126414,
		.end_address = 127618 
	},
	{ 
		.item_name = "icon-s-run-log.bin",
		.start_address = 127618,
		.end_address = 128822 
	},
	{ 
		.item_name = "icon-s-sleep.bin",
		.start_address = 128822,
		.end_address = 130026 
	},
	{ 
		.item_name = "icon_setting.bin",
		.start_address = 130026,
		.end_address = 131758 
	},
};
size_t bin_infos_len = 14;