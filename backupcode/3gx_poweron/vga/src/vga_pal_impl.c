#include "pal_display.h"
#include "vga.h"

s_display_dev display_vga_dev =
{
  .dev_name   = "vga",
  .open       = vga_open,
  .update     = vga_update,
  .print_text = vga_print_text
};


s_display_dev* soc_get_display_dev(void)
{
  return &display_vga_dev;
}

