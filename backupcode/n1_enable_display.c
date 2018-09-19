soclib/devices/pm/power_control/pow/src/board_sf_3gx_ext_pmic/pow_bootcore.c
void pmu_bootcore_enable_backlight(void)
{
#if 1 //pwm2 
	/* 3gr mrd p2.0 pwm2 is used as backlight controle */
       /* pcl_55 set as pwm2 */
       *((volatile unsigned int *)(0xE4600000 + 0x200 + 55*4)) = 0x1410;

       /* pwm2 period/duty/config */
       *((volatile unsigned int *)(0xE1A00020 + 0x04)) = 0x28A;
	*((volatile unsigned int *)(0xE1A00020 + 0x08)) = 0x143;
	*((volatile unsigned int *)(0xE1A00020 + 0x0C)) = 0x0B;  
#else
#if !defined (AG620_ES2_0_DEFINED_FOR_BL)
    pow_iowrite(0xE650092C, 0xA00);

    cgu_backlight_clk_enable();

    pow_iowrite(0xE650090C, 0xFF);
    pow_iowrite(0xE6500908, 0xFB);
    pow_iowrite(0xE6500918, 0xFFFF);
    pow_iowrite(0xE650092C, 0x116);
#else
    pow_iowrite(0xE650182C, 0x200); //LED_CTRL
    pow_iowrite(0xE6501954, 0x800); //safe_LED_CTRL

    cgu_backlight_clk_enable();

  #if defined(SF_ES_2_0)
    pow_iowrite(0xE650180C, 0x00000127);
    pow_iowrite(0xE6501808, 0x00000122);
  #else
    pow_iowrite(0xE650180C, 0xFF);
    pow_iowrite(0xE6501808, 0xFB);
  #endif
    pow_iowrite(0xE650182C, 0x104);
    pow_iowrite(0xE6501954, 0x0012); //GPOenable, CLSL default; ref_mux_sel - 10, mode -10, LED_STBY don't care
#endif
#endif
}

bootsystems/platforms/src/sf_3gx_soc/board_sf_3gx/platform_power.c
BOOL platform_power_set_backlight(U32 value)
{
    pmu_bootcore_enable_backlight();
  return TRUE;
}

bootsystems/applications/slb/src/slb_main.c
  TRACE_BC_PRINTF("------------- Starting SLB -------------");
  //display
  extern unsigned int g_cgu_dcc_clk_rate;
  g_cgu_dcc_clk_rate = 297000000;
  
	*((volatile int *)0xe4700a3c) = 0x1c;
  //     bl_display_fp.bl_lcd_power_on();
  power_bootsystem_set(POWER_BOOTSYSTEM_DISPLAY_POWER, 1);
  //     bl_display_fp.bl_dcc_power_on();
  power_bootsystem_set(POWER_BOOTSYSTEM_DISPLAY_CLOCK, 1);
  *((volatile int *)0xe4700a3c) = 0x10;
  power_bootsystem_set(POWER_BOOTSYSTEM_BACKLIGHT, 1);
  slb_linux_append_cmdline("panelsource=1");


bootsystems/drivers/display/src/display_img_drv.c
T_ENUM_DISPLAY_DRV_RETURN_TYPE display_drv_show_image(T_ENUM_SELECT_IMG img)
{
  T_SPLASH_IMAGE_HDR *config_header;
  U32 img_size;
  U8 *img_buf;

    return DISPLAY_DRV_RETURN_TYPE_OK;
  display_drv_platform_init();
