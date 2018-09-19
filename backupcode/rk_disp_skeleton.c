/*rockchip_disp_drv.c */
fs_initcall(rockchip_disp_init);
static int __init rockchip_disp_init(void) {
	platform_driver_register(&rockchip_fb_driver);
	platform_driver_register(&rockchip_screen_driver);
	platform_driver_register(&xgold_mipi_dsi_driver);
	platform_driver_register(&rockchip_vop_driver);
	platform_driver_register(&rockchip_disp_platform_driver);
}

/*rockchip_fb.c */
static int rockchip_fb_probe(struct platform_device *pdev) {
	of_property_read_u32(np, "rockchip,loader-logo-on", &loader_logo_on);
	of_property_read_string(np, "rockchip,ion-drv", &string);

	if (sfb_info->ion_server_type == ION_DRV_RK)
		sfb_info->ion_client = rockchip_ion_client_create("rockchip-fb");
}
int rockchip_fb_register(struct rockchip_vop_driver *dev_drv,
		     struct rockchip_vop_win *vop_win, int id)
{
	init_vop_device_driver(dev_drv, vop_win, id);

	for (i = 0; i < dev_drv->num_win; i++) {
		fbi = framebuffer_alloc(0, &fb_pdev->dev);
		ret = register_framebuffer(fbi);
		rockchip_fb_create_sysfs(fbi);
	}
	if (dev_drv->prop == PRMRY) {
		main_fbi->fbops->fb_open(main_fbi, 1);
		rockchip_fb_alloc_buffer(main_fbi, 0);
		if (dev_drv->iommu_enabled) {
			if (dev_drv->mmu_dev)
				rockchip_iovmm_set_fault_handler(dev_drv->dev,
					rockchip_fb_sysmmu_fault_handler);
		}
		main_fbi->fbops->fb_set_par(main_fbi);
	}
}

static int init_vop_device_driver(struct rockchip_vop_driver *dev_drv,
				  struct rockchip_vop_win *def_win, int id)
{
	if (dev_drv->prop == PRMRY) {
		rockchip_get_prmry_screen(screen);
	}
	dev_drv->trsm_ops = rockchip_fb_trsm_ops_get(screen->type);
}
static struct fb_ops fb_ops = {
	.owner = THIS_MODULE,
	.fb_open = rockchip_fb_open,
	.fb_release = rockchip_fb_close,
	.fb_check_var = rockchip_fb_check_var,
	.fb_set_par = rockchip_fb_set_par,
	.fb_blank = rockchip_fb_blank,
	.fb_ioctl = rockchip_fb_ioctl,
	.fb_pan_display = rockchip_fb_pan_display,
	.fb_read = rockchip_fb_read,
	.fb_write = rockchip_fb_write,
	.fb_setcolreg = fb_setcolreg,
	.fb_fillrect = cfb_fillrect,
	.fb_copyarea = cfb_copyarea,
	.fb_imageblit = cfb_imageblit,
};


/* rockchip_screen.c */
static int rockchip_screen_probe(struct platform_device *pdev)
{
	for_each_compatible_node(display_np, NULL, PROP_DISPLAY) {
		screen = rockchip_prase_screen_dt(display_np, pdev);
		if (screen) {
			screen->index = index;
			screen->power_on = rockchip_screen_power_on;
			screen->power_off = rockchip_screen_power_off;
			screen->pm_platdata = pm_platdata;
			list_add_tail(&screen->panel_list, &screen_list);

			if (panel_source < 0 && screen->index == 0)
				screen->index = panel_source;
			if (screen->index == panel_source)
				cur_screen = screen;
		}
		index++;
	}
}

static struct rockchip_screen *
rockchip_prase_screen_dt(struct device_node *np, struct platform_device *pdev) {
	of_get_display_timings(np);
	display_timings_get(disp_timing, disp_timing->native_mode);
	rockchip_fb_videomode_from_timing(dt, screen);
	rockchip_disp_pwr_ctr_parse_dt(np, screen);
	display_timings_release(disp_timing);
}

/* xgold/dsi/dsi_device.c */
static const struct of_device_id xgold_mipi_dsi_dt_ids[] = {
	{.compatible = "intel,xgold-mipi_dsi",},
	{}
};
struct platform_driver xgold_mipi_dsi_driver = {
	.probe = xgold_mipi_dsi_probe,
};
static struct rockchip_fb_trsm_ops trsm_mipi_dsi_ops = {
	.enable = xgold_mipi_dsi_enable,
	.disable = xgold_mipi_dsi_disable,
	.detect_panel = xgold_mipi_dsi_detect_panel,
};
static int xgold_mipi_dsi_probe(struct platform_device *pdev)
{
	rockchip_get_prmry_screen(&mipi_dsi->screen);
	res = platform_get_resource_byname(pdev, IORESOURCE_MEM, "mipi_dsi_phy");
	mipi_dsi->regbase = devm_ioremap_resource(&pdev->dev, res);
	mipi_dsi->irq.err = platform_get_irq(pdev, 0);
	rockchip_fb_trsm_ops_register(&trsm_mipi_dsi_ops, SCREEN_MIPI);
	dsi_of_parse_display(pdev, mipi_dsi);
	dsi_probe(mipi_dsi);
	dsi_irq_probe(mipi_dsi);
}

/* rockchip_vop.c */
static const struct of_device_id rockchip_vop_dt_ids[] = {
	{ .compatible = "rockchip,vop", },
	{ },
};
static int rockchip_vop_probe(struct platform_device *pdev) {
	rockchip_vop_parse_dt(vop_dev);
	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
	vop_dev->reg_phy_base = res->start;
	vop_dev->len = resource_size(res);
	vop_dev->regs = devm_ioremap_resource(dev, res);
	dev_drv->ops = &vop_drv_ops;
	vop_dev->pm_platdata = of_device_state_pm_setup(dev->of_node);
	device_state_pm_set_class(&pdev->dev, vop_dev->pm_platdata->pm_user_name);
	rockchip_fb_register(dev_drv, vop_win, vop_dev->id);
}
static struct rockchip_vop_drv_ops vop_drv_ops = {
	.open = rockchip_vop_open,
	.load_screen = rockchip_vop_load_screen,
	.set_par = rockchip_vop_set_par,
	.pan_display = rockchip_vop_pan_display,
	.direct_set_addr = rockchip_vop_direct_set_win_addr,
	.blank = rockchip_vop_blank,
	.ioctl = rockchip_vop_ioctl,
	.get_win_state = rockchip_vop_get_win_state,
	.ovl_mgr = rockchip_vop_ovl_mgr,
	.get_disp_info = rockchip_vop_get_disp_info,
	.fps_mgr = rockchip_vop_fps_mgr,
	.fb_get_win_id = rockchip_vop_get_win_id,
	.fb_win_remap = rockchip_fb_win_remap,
	.poll_vblank = rockchip_vop_poll_vblank,
	.get_dsp_addr = rockchip_vop_get_dsp_addr,
	.cfg_done = rockchip_vop_cfg_done,
	.dump_reg = rockchip_vop_reg_dump,
	.set_dsp_bcsh_hue = rockchip_vop_set_bcsh_hue,
	.set_dsp_bcsh_bcs = rockchip_vop_set_bcsh_bcs,
	.get_dsp_bcsh_hue = rockchip_vop_get_bcsh_hue,
	.get_dsp_bcsh_bcs = rockchip_vop_get_bcsh_bcs,
	.open_bcsh = rockchip_vop_open_bcsh,
	.set_dsp_lut = rockchip_vop_set_lut,
	.set_hwc_lut = rockchip_vop_set_hwc_lut,
	.set_irq_to_cpu = rockchip_vop_set_irq_to_cpu,
	.lcdc_reg_update = rockchip_vop_reg_update,
	.mmu_en = rockchip_vop_mmu_en,
	.reg_writel = rockchip_vop_reg_writel,
	.reg_readl = rockchip_vop_reg_readl,
};
