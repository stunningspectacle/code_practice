/*  -------------------------------------------------------------------------
    Copyright (C) 2013-2014 Intel Mobile Communications GmbH

         Sec Class: Intel Confidential (IC)
    ----------------------------------------------------------------------
    Revision Information:
       $File name:  /dwdtoolsrc/bootcore/drivers/display/inc/display_drv.h $
  ---------------------------------------------------------------------- */

#ifndef __DISPLAY_DRV_H__
#define __DISPLAY_DRV_H__

/*****************************************************************************/
/* INCLUDES                                                                  */
/*****************************************************************************/
#include <bastypes.h>

#define SPLASH_IMAGE_MAGIC "!SPLASH!"
#define SPLASH_MAGIC_SIZE   32
#define MAX_SUPPORT_PANEL   16

/*****************************************************************************/
/* TYPE DEFINES                                                              */
/*****************************************************************************/
typedef enum ENUM_SELECT_IMG
{
  SELECT_IMG_NO_DSIPLAY    = 0,
  SELECT_IMG_CHARGE_ONLY   = 1,
  SELECT_IMG_SPLASH_SCREEN = 1,
  SELECT_IMG_FASTBOOT,
  SELECT_IMG_FONT,
  SELECT_IMG_NUMBER_OF
} T_ENUM_SELECT_IMG;

typedef enum ENUM_IMG_COMPRESS_TYPE
{
  IMG_COMPRESS_TYPE_NO_COMPRESS = 0,
  IMG_COMPRESS_TYPE_RLE,
  IMG_COMPRESS_TYPE_NUMBER_OF
} T_ENUM_IMG_COMPRESS_TYPE;

typedef enum ENUM_IMG_TYPE
{
  IMG_TYPE_RGB565 = 0,
  IMG_TYPE_RGB888,
  IMG_TYPE_RGBA8888,
  IMG_TYPE_BGRX8888,
  IMG_TYPE_NUMBER_OF
} T_ENUM_IMG_TYPE;

typedef enum ENUM_DISPLAY_DRV_RETURN_TYPE
{
  DISPLAY_DRV_RETURN_TYPE_OK,
  DISPLAY_DRV_RETURN_TYPE_FAIL,
  DISPLAY_DRV_RETURN_TYPE_UNKNOWN_IMG
} T_ENUM_DISPLAY_DRV_RETURN_TYPE;

typedef struct {
    U8      magic[SPLASH_MAGIC_SIZE];
    U32     height[MAX_SUPPORT_PANEL];
    U32     width[MAX_SUPPORT_PANEL];
    U32     bytes_per_pixel;
    U32     font_height;  //small font height
    U32     font_width;   //small font width
    U32     font_character_count; //total characters in the font database
    U8      padding[512 - ( 32 + 2*4*16 + 4 + 3*4) ]; //total size should be 512 bytes
} T_SPLASH_IMAGE_HDR;

typedef struct {
	T_SPLASH_IMAGE_HDR splash_img_hdr;
	U32 display_config[];
}T_SPASH_CONFIG;

typedef enum ENUM_COLOR
{
 WHITE,
 RED,
 GREEN,
 BLUE,
 YELLOW
} T_ENUM_COLOR;
/*****************************************************************************/
/* FUNCTION PROTOTYPES                                                       */
/*****************************************************************************/
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_update_screen(T_ENUM_SELECT_IMG img);
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_show_icon(int img_x, int img_y, int img_w, int img_h, int x, int y);
void display_drv_init(void);
void display_drv_free(void);
void display_drv_clear_screen(void);
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_apply_screen(void);
void display_drv_update_string_at_pos_with_color(U8 * idx,U32 len,U32 x,U32 y,T_ENUM_COLOR color);
U32 display_drv_get_screen_width(void);
U32 display_drv_get_screen_height(void);
U32 display_drv_get_screen_bpp(void);
U32 display_drv_get_font_width(void);
U32 display_drv_get_font_height(void);
U32 display_drv_get_font_character_count(void);
extern void display_backlight_control(int cmd);
#endif // __DISPLAY_DRV_H__
