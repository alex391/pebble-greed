#include "pebble_process_info.h"
#include "src/resource_ids.auto.h"

const PebbleProcessInfo __pbl_app_info __attribute__ ((section (".pbl_header"))) = {
  .header = "PBLAPP",
  .struct_version = { PROCESS_INFO_CURRENT_STRUCT_VERSION_MAJOR, PROCESS_INFO_CURRENT_STRUCT_VERSION_MINOR },
  .sdk_version = { PROCESS_INFO_CURRENT_SDK_VERSION_MAJOR, PROCESS_INFO_CURRENT_SDK_VERSION_MINOR },
  .process_version = { 1, 0 },
  .load_size = 0xb6b6,
  .offset = 0xb6b6b6b6,
  .crc = 0xb6b6b6b6,
  .name = "greed",
  .company = "MakeAwesomeHappen",
  .icon_resource_id = DEFAULT_MENU_ICON,
  .sym_table_addr = 0xA7A7A7A7,
  .flags = PROCESS_INFO_PLATFORM_GABBRO,
  .num_reloc_entries = 0xdeadcafe,
  .uuid = { 0x0E, 0x81, 0x50, 0xDA, 0xEF, 0xD1, 0x40, 0xA8, 0xBB, 0x5C, 0x06, 0x04, 0x42, 0x42, 0x7E, 0xBE },
  .virtual_size = 0xb6b6
};
