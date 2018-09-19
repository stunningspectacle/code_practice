/* ----------------------------------------------------------------------------

   Copyright (C) 2013 Intel Mobile Communications GmbH

        Sec Class: Intel Confidential (IC)

   All rights reserved.
   ----------------------------------------------------------------------------

   This document contains proprietary information belonging to IMC.
   Technologies Denmark A/S. Passing on and copying of this document, use
   and communication of its contents is not permitted without prior written
   authorisation.

   Revision Information:
  $File name:  /dwdtoolsrc/bootcore/drivers/display/src/display_drv.c $
  ---------------------------------------------------------------------------*/

/*****************************************************************************/
/* INCLUDES                                                                  */
/*****************************************************************************/
#include <stdbool.h>
#include <display_drv.h>
#include <platform_drv.h>
#include <stm_bootcore.h>
#include <mmu_bootcore.h>
#include <power_bootsystem.h>
#include <storage_al.h>
#include <ftle_def.h>
#include <bl-dcc.h>
#include <pcl_bootsystem.h>
#include <trace_bootcore.h>

#define SPALSH_IMAGE_HEADER_SIZE_IN_BYTES 512
#define DISPLAY_CONFIG_DATA_SIZE_IN_BYTES (100*1024)
#define SPLASH_CONFIG_DATA_SIZE_IN_512_BLOCKS ((DISPLAY_CONFIG_DATA_SIZE_IN_BYTES + SPALSH_IMAGE_HEADER_SIZE_IN_BYTES)/512)
#define SPALSH_IMAGE_SIZE_IN_BYTES (10*1024*1024)
#define FONT_IMAGE_MAX_LEN   0x6C000
#define CLEAR_RGBA 0xFF000000
#define BYTES_PER_PIXEL 4
#define FONT_KIND_COUNT 2

extern BOOL onkey_status;
static BOOL is_backlight_enabled = FALSE;
static T_ENUM_SELECT_IMG display_showing_img;
static BOOL is_font_loaded = FALSE;
static BOOL is_driver_inited =  FALSE;
static U32  font_data_size = FONT_IMAGE_MAX_LEN;
static U32 display_drv_get_font_data_size(void);
static BOOL display_driver_init;

static void display_disable_phy_isolation(void)
{
  power_bootsystem_set(POWER_BOOTSYSTEM_DISPLAY_PHY_ISOLATION, 0);
}

static void display_set_reset_pin_low(void)
{
  pcl_bootsystem_set(PCL_BOOTSYSTEM_USER_DISPLAY_RESET, FALSE);
}

static void display_set_reset_pin_high(void)
{
  pcl_bootsystem_set(PCL_BOOTSYSTEM_USER_DISPLAY_RESET, TRUE);
}

void display_backlight_control(int cmd)
{
  power_bootsystem_set(POWER_BOOTSYSTEM_BACKLIGHT, (U32)cmd);
}

static void display_clock_enable(void)
{
  power_bootsystem_set(POWER_BOOTSYSTEM_DISPLAY_CLOCK, 1);
}

static void display_power_enable(void)
{
  power_bootsystem_set(POWER_BOOTSYSTEM_DISPLAY_POWER, 1);
}


bl_display_cbs_t bl_display_fp =
{
  display_clock_enable,
  display_power_enable,
  display_disable_phy_isolation,
  display_set_reset_pin_low,
  display_set_reset_pin_low,
  display_set_reset_pin_high,
  display_backlight_control,
  stm_bootcore_wait_ms,
  cache_clean_datacache_range,
};


/*****************************************************************************/
/* LOCAL DATA                                                                */
/*****************************************************************************/

__SECTION("DISPLAY_BUFFER") __align(32) U8 splash_config_data[SPALSH_IMAGE_HEADER_SIZE_IN_BYTES + DISPLAY_CONFIG_DATA_SIZE_IN_BYTES];
__SECTION("DISPLAY_BUFFER") __align(32) U8 img_raw_buf[SPALSH_IMAGE_SIZE_IN_BYTES] = {0};
__SECTION("DISPLAY_BUFFER") __align(32) U8 font_raw_buf[FONT_IMAGE_MAX_LEN]={0};

static U8 *icon_raw_buf;
static unsigned char index = 1;

/*****************************************************************************/
/* EXPORTED FUNCTIONS                                                        */
/*****************************************************************************/
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_update_screen(T_ENUM_SELECT_IMG img)
{
#if defined(DISPLAY_DRIVER_SUPPORT)
  bl_dcc_result_type error;
  T_SPASH_CONFIG* splash_config;
  U32 i, splash_image_size;
  U32 start_block = 0;
  U32 hw_id = BL_DCC_PLATFORM_FF;
#endif
  if(!display_driver_init)
  {
    display_driver_init = TRUE;
#if defined (SF_3G_R_MRD6S)
    index = platform_display_init();
#else
		platform_display_init();
#endif
  }

  if(img == SELECT_IMG_NO_DSIPLAY)
    return DISPLAY_DRV_RETURN_TYPE_OK;
#if 0
  if(img == display_showing_img)
    return DISPLAY_DRV_RETURN_TYPE_OK;
#endif
  display_showing_img = img;

  if(img >= SELECT_IMG_NUMBER_OF)
    return DISPLAY_DRV_RETURN_TYPE_UNKNOWN_IMG;

#if defined(DISPLAY_DRIVER_SUPPORT)
  TRACE_BC_PRINTF("%s: start panel index:%d\n", __func__, index);

  if(!storage_al_ftle_lcd_open())
  {
    return DISPLAY_DRV_RETURN_TYPE_FAIL;
  }
  //Load splash image header and display config Data
  storage_al_ftle_read_bytes(start_block, SPALSH_IMAGE_HEADER_SIZE_IN_BYTES + DISPLAY_CONFIG_DATA_SIZE_IN_BYTES, &splash_config_data[0], FTLE_ID_STARTUP_IMAGE);
  start_block += SPLASH_CONFIG_DATA_SIZE_IN_512_BLOCKS; // increase to skip the display config data

  splash_image_size = (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel);
  for(i = SELECT_IMG_SPLASH_SCREEN; i < (U32)img; i++)
  {
    start_block += (splash_image_size % 512) ? (splash_image_size / 512 + 1) : (splash_image_size / 512);
  }
  if(!storage_al_ftle_read_bytes(start_block, splash_image_size, img_raw_buf, FTLE_ID_STARTUP_IMAGE))
  {
    return DISPLAY_DRV_RETURN_TYPE_FAIL;
  }

  /* osless_dcc lib interface */
  error = bl_dcc_show_image(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index],
                            ((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index],
                            ((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel,
                            img_raw_buf,
                            &bl_display_fp,
                            (bl_lcd_config_t*) & (((T_SPASH_CONFIG *)splash_config_data)->display_config[0]),
                            hw_id, index);
  TRACE_BC_PRINTF("%s: ret=%d\n", __func__, error);

  if(!is_backlight_enabled)
  {
    if(onkey_status == FALSE)
      display_backlight_control(1);
    is_backlight_enabled = TRUE;
  }
#endif
  return DISPLAY_DRV_RETURN_TYPE_OK;
}

//init the display driver
//mainly load the configuration data including font
void display_drv_init(void)
{
	U32 i, splash_image_size, splash_img_block;
	U32 start_block = 0;

	storage_al_ftle_read_bytes(start_block, SPALSH_IMAGE_HEADER_SIZE_IN_BYTES + DISPLAY_CONFIG_DATA_SIZE_IN_BYTES, &splash_config_data[0],FTLE_ID_STARTUP_IMAGE);
	start_block +=SPLASH_CONFIG_DATA_SIZE_IN_512_BLOCKS; // increase to skip the display config data

	splash_image_size = (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel);
	splash_img_block = (splash_image_size % 512) ? (splash_image_size / 512 + 1) : (splash_image_size / 512);

	for(i =SELECT_IMG_SPLASH_SCREEN; i < (U32)SELECT_IMG_FONT; i++)
	{
		start_block += splash_img_block;
	}

	//TRACE_BC_PRINTF("start_block font  = %d\n",start_block);
	font_data_size = display_drv_get_font_data_size();
	storage_al_ftle_read_bytes(start_block, FONT_IMAGE_MAX_LEN, font_raw_buf,FTLE_ID_STARTUP_IMAGE);
	is_font_loaded = TRUE;
	is_driver_inited = TRUE;
}

static U8 *display_drv_load_icons(void)
{
	U32 i, splash_image_size, start_block;

	if (icon_raw_buf)
		return icon_raw_buf;

	splash_image_size = (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]) * (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel);

	icon_raw_buf = (U8 *)malloc(SPALSH_IMAGE_SIZE_IN_BYTES);

	if (!icon_raw_buf)
	{
		TRACE_BC_PRINTF("Error : Could not allocate Icons (s=%d)",SPALSH_IMAGE_SIZE_IN_BYTES);
		return NULL;
	}

	start_block = SPLASH_CONFIG_DATA_SIZE_IN_512_BLOCKS; // increase to skip the display config data
	for(i = SELECT_IMG_SPLASH_SCREEN; i < SELECT_IMG_FASTBOOT; i++)
	{
		start_block += (splash_image_size % 512) ? (splash_image_size / 512 + 1) : (splash_image_size / 512);
	}
	storage_al_ftle_read_bytes(start_block, SPALSH_IMAGE_SIZE_IN_BYTES, icon_raw_buf, FTLE_ID_STARTUP_IMAGE);
	return icon_raw_buf;
}

void display_drv_free(void)
{
	if (icon_raw_buf)
	{
		free(icon_raw_buf);
		icon_raw_buf = NULL;
	}
}

// clear display screen
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_show_icon(int img_x, int img_y, int img_w, int img_h, int x, int y)
{
	U32 splash_image_pixels = 0;
	U32 i = 0;
	U32 ix, iy;
	U32 *img_ptr = (U8 *)img_raw_buf;
	U32 *icon_ptr;
	U32 start_block;
	U32 splash_image_size = 0;

	icon_ptr = (U32 *)display_drv_load_icons();

	if (!icon_ptr)
	{
		TRACE_BC_PRINTF("Error : Icons not loaded");
		return DISPLAY_DRV_RETURN_TYPE_FAIL;
	}

	for(iy=0;iy<img_h;iy++)
		for(ix=0;ix<img_w;ix++)
		{
			img_ptr[((y+iy)*(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index])+x+ix)] =
                               icon_ptr[((img_y+iy)*(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index])+img_x+ix)];
		}

	return DISPLAY_DRV_RETURN_TYPE_OK;
}

void display_drv_clear_screen(void)
{
	U32 splash_image_pixels = 0;
	U32 i = 0;
	U32 * buf_ptr = (U32 *)img_raw_buf;
	if(!display_driver_init)
	{
		display_driver_init = TRUE;
#if defined (SF_3G_R_MRD6S)
		index = platform_display_init();
#else
		platform_display_init();
#endif
	}
	display_drv_init();
	splash_image_pixels = (((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index])
		*(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]);

    for(i=0;i<splash_image_pixels;i++)
    {
        buf_ptr[i] = CLEAR_RGBA;
    }
}

// add display_drv_apply_screen funtion

T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_apply_screen()
{
  bl_dcc_result_type error;
  U32 hw_id = BL_DCC_PLATFORM_FF;
  //cache_clean_all();
  /* osless_dcc lib interface */
  error = bl_dcc_show_image(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index],
                            ((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index],
                            ((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel,
                            img_raw_buf,
                            &bl_display_fp,
                            (bl_lcd_config_t*)&(((T_SPASH_CONFIG *)splash_config_data)->display_config[0]),
                            hw_id,index);
  if(!is_backlight_enabled)
  {
    /* Enable backlight only after the image update on the screen */
    display_backlight_control(1);
    is_backlight_enabled = TRUE;
  }
  return DISPLAY_DRV_RETURN_TYPE_OK;
}

//apply color on the font pixels
//just very basic RGB convertion
static void change_color(U8 * img, U32 len, T_ENUM_COLOR color)
{
    U32 i;

    switch (color)
    {
	    case RED:
	    for(i=0;i<len;i=i+(BYTES_PER_PIXEL))
            {
                img[i+1] = '\0';
                img[i+2]= '\0';
            }
            break;
            case GREEN:
            for(i=0;i<len;i=i+(BYTES_PER_PIXEL))
            {
                img[i+0] = '\0';
                img[i+2]= '\0';
            }
            break;
	    case BLUE:
            for(i=0;i<len;i=i+(BYTES_PER_PIXEL))
            {
                img[i+0] = '\0';
                img[i+1]= '\0';
            }
	    break;
	    case YELLOW:
            for(i=0;i<len;i=i+(BYTES_PER_PIXEL))
            {
                img[i+2] = '\0';
            }
            break;
	    WHITE:
	    default:
	      //do nothing
	    break;
     }
}

//idx is a array of int which has length len
//each member of the array is actually a index of the character to be displayed, currently index is in [0,95],
//totally 96 characters are being supported
//display the given string at pos (x,y) with an img as background image
void display_drv_update_string_at_pos_with_color(U8 * idx,U32 len,U32 x,U32 y,T_ENUM_COLOR color)
{
	U32 start_pos_img =  0;
    U32 start_pos_font = 0;
    U32 i, j;
    U32 screen_width;
    U32 bpp;
    U32 font_width, font_height,font_character_count;
    U32 c1,c2,c3;

    screen_width = display_drv_get_screen_width();
    bpp = display_drv_get_screen_bpp();
    font_width = display_drv_get_font_width();
    font_height = display_drv_get_font_height();
    font_character_count = display_drv_get_font_character_count();
    c1 = bpp*font_width;
    c2 = c1 *font_character_count;
    c3 = screen_width * bpp;
    //modify img_raw_buf with the characters font data
    //superimpose characters at x,y
    for(i=0;i<len;i++)
    {
        start_pos_img = (y*screen_width+x)*bpp;
        start_pos_font = (idx[i]*font_width)*bpp;

        for(j=0;j<font_height;j++)
        {
            memcpy(&img_raw_buf[start_pos_img],&font_raw_buf[start_pos_font],c1);
            change_color(&img_raw_buf[start_pos_img],c1,color);
            start_pos_font=start_pos_font + c2;
            start_pos_img=start_pos_img + c3;
        }

        x=x+font_width;
    }
}

//get screen width in pixels
U32 display_drv_get_screen_width(void)
{
  //TRACE_BC_PRINTF("display_drv_get_screen_width ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.width[index]);
}

//get screen height in pixels
U32 display_drv_get_screen_height(void)
{
  //TRACE_BC_PRINTF("display_drv_get_screen_height ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index]));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.height[index]);
}

//get screen bytes per pixel
U32 display_drv_get_screen_bpp(void)
{
  //TRACE_BC_PRINTF("display_drv_get_screen_bpp ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.bytes_per_pixel);
}

//get screen font width
U32 display_drv_get_font_width(void)
{
  //TRACE_BC_PRINTF("display_drv_get_font_width ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_width));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_width);
}

//get screen font width
U32 display_drv_get_font_height(void)
{
  //TRACE_BC_PRINTF("display_drv_get_font_height ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_height));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_height);
}

//get screen font width
U32 display_drv_get_font_character_count(void)
{
  //TRACE_BC_PRINTF("display_drv_get_font_character_count ret=%d\n",(U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_character_count));
    return  (U32)(((T_SPASH_CONFIG *)splash_config_data)->splash_img_hdr.font_character_count);
}

//get screen font data size
static U32 display_drv_get_font_data_size(void)
{
    U32 font_width = display_drv_get_font_width();
    U32 font_height = display_drv_get_font_height();
    U32 font_character_count = display_drv_get_font_character_count();
    U32 bpp = display_drv_get_screen_bpp();
    return  (font_width*font_height*font_character_count*bpp*(FONT_KIND_COUNT));
}
