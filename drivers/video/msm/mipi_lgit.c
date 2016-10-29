/*
 *  Copyright (C) 2011-2012, LG Eletronics,Inc. All rights reserved.
 *      LGIT LCD device driver
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */
#include <linux/string.h>
#include <linux/gpio.h>
#include <linux/syscore_ops.h>
#include <linux/display_state.h>

#include "msm_fb.h"
#include "mipi_dsi.h"
#include "mipi_lgit.h"
#include "mdp4.h"

static struct msm_panel_common_pdata *mipi_lgit_pdata;

static struct dsi_buf lgit_tx_buf;
static struct dsi_buf lgit_rx_buf;
static struct msm_fb_data_type *local_mfd;
static int skip_init;

struct dsi_cmd_desc new_color_vals[33];

#define DSV_ONBST 57

bool display_on = true;

bool is_display_on()
{
	return display_on;
}

static int lgit_external_dsv_onoff(uint8_t on_off)
{
	int ret =0;
	static int init_done=0;

	if (!init_done) {
		ret = gpio_request(DSV_ONBST,"DSV_ONBST_en");
		if (ret) {
			pr_err("%s: failed to request DSV_ONBST gpio \n", __func__);
			goto out;
		}
		ret = gpio_direction_output(DSV_ONBST, 1);
		if (ret) {
			pr_err("%s: failed to set DSV_ONBST direction\n", __func__);
			goto err_gpio;
		}
		init_done = 1;
	}

	gpio_set_value(DSV_ONBST, on_off);
	mdelay(20);
	goto out;

err_gpio:
	gpio_free(DSV_ONBST);
out:
	return ret;
}

static int mipi_lgit_lcd_on(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	int ret = 0;

	pr_info("%s started\n", __func__);

	display_on = true;

	mfd = platform_get_drvdata(pdev);
	local_mfd = mfd;
	if (!mfd)
		return -ENODEV;
	if (mfd->key != MFD_KEY)
		return -EINVAL;

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			new_color_vals,
			mipi_lgit_pdata->power_on_set_size_1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_on_set_1 cmds\n", __func__);
		return ret;
	}

	if(!skip_init){
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
		ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
				mipi_lgit_pdata->power_on_set_2,
				mipi_lgit_pdata->power_on_set_size_2);
		MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
		if (ret < 0) {
			pr_err("%s: failed to transmit power_on_set_2 cmds\n", __func__);
			return ret;
		}
	}
	skip_init = false;

	ret = lgit_external_dsv_onoff(1);
	if (ret < 0) {
		pr_err("%s: failed to turn on external dsv\n", __func__);
		return ret;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_on_set_3,
			mipi_lgit_pdata->power_on_set_size_3);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_on_set_3 cmds\n", __func__);
		return ret;
	}

	pr_info("%s finished\n", __func__);
	return 0;
}

static int mipi_lgit_lcd_off(struct platform_device *pdev)
{
	struct msm_fb_data_type *mfd;
	int ret = 0;

	pr_info("%s started\n", __func__);

	display_on = false;

	if (mipi_lgit_pdata->bl_pwm_disable)
		mipi_lgit_pdata->bl_pwm_disable();

	mfd = platform_get_drvdata(pdev);

	if (!mfd)
		return -ENODEV;

	if (mfd->key != MFD_KEY)
		return -EINVAL;

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_1,
			mipi_lgit_pdata->power_off_set_size_1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_off_set_1 cmds\n", __func__);
		return ret;
	}

	ret = lgit_external_dsv_onoff(0);
	if (ret < 0) {
		pr_err("%s: failed to turn off external dsv\n", __func__);
		return ret;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_2,
			mipi_lgit_pdata->power_off_set_size_2);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_off_set_2 cmds\n", __func__);
		return ret;
	}

	pr_info("%s finished\n", __func__);
	return 0;
}

static void mipi_lgit_lcd_shutdown(void)
{
	int ret = 0;

	if(local_mfd && !local_mfd->panel_power_on) {
		pr_info("%s:panel is already off\n", __func__);
		return;
	}

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_1,
			mipi_lgit_pdata->power_off_set_size_1);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_off_set_1 cmds\n", __func__);
	}

	ret = lgit_external_dsv_onoff(0);
	if (ret < 0) {
		pr_err("%s: failed to turn off external dsv\n", __func__);
	}
	mdelay(20);

	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x10000000);
	ret = mipi_dsi_cmds_tx(&lgit_tx_buf,
			mipi_lgit_pdata->power_off_set_2,
			mipi_lgit_pdata->power_off_set_size_2);
	MIPI_OUTP(MIPI_DSI_BASE + 0x38, 0x14000000);
	if (ret < 0) {
		pr_err("%s: failed to transmit power_off_set_2 cmds\n", __func__);
	}

	pr_info("%s finished\n", __func__);
}

static int mipi_lgit_backlight_on_status(void)
{
	return (mipi_lgit_pdata->bl_on_status());
}

static void mipi_lgit_set_backlight_board(struct msm_fb_data_type *mfd)
{
	int level;

	level = (int)mfd->bl_level;
	mipi_lgit_pdata->backlight_level(level, 0, 0);
}

/******************* begin faux123 sysfs interface *******************/
static bool calc_checksum(int intArr[]) {
	int i = 0;
	unsigned char chksum = 0;

	if (intArr[5] > 31 || (intArr[6] > 31)) {
		pr_info("gamma 0 and gamma 1 values can't be over 31, got %d %d instead!", intArr[5], intArr[6]);
		return false;
	}

	for (i=1; i<10; i++) {
		if (intArr[i] > 255) {
			pr_info("color values  can't be over 255, got %d instead!", intArr[i]);
			return false;
		}
		chksum += intArr[i];
	}

	if (chksum == (unsigned char)intArr[0]) {
		return true;
	} else {
		pr_info("expecting %d, got %d instead!", chksum, intArr[0]);
		return false;
	}
}

static ssize_t kgamma_r_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum(kgamma)) {
		kgamma[0] = 0xd0;
		for (i=0; i<10; i++) {
			pr_info("kgamma_r_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[5].payload[i] = kgamma[i];
		}

		kgamma[0] = 0xd1;
		for (i=0; i<10; i++) {
			pr_info("kgamma_r_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[6].payload[i] = kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_r_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	int kgamma[10];
	int i;

	for (i=0; i<10; i++)
		kgamma[i] = new_color_vals[5].payload[i];

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d", 
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static ssize_t kgamma_g_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum(kgamma)) {
		kgamma[0] = 0xd2;
		for (i=0; i<10; i++) {
			pr_info("kgamma_g_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[7].payload[i] = kgamma[i];
		}

		kgamma[0] = 0xd3;
		for (i=0; i<10; i++) {
			pr_info("kgamma_g_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[8].payload[i] = kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_g_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	int kgamma[10];
	int i;

	for (i=0; i<10; i++)
		kgamma[i] = new_color_vals[7].payload[i];

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d", 
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static ssize_t kgamma_b_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum(kgamma)) {
		kgamma[0] = 0xd4;
		for (i=0; i<10; i++) {
			pr_info("kgamma_b_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[9].payload[i] = kgamma[i];
		}

		kgamma[0] = 0xd5;
		for (i=0; i<10; i++) {
			pr_info("kgamma_b_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[10].payload[i] = kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_b_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	int kgamma[10];
	int i;

	for (i=0; i<10; i++)
		kgamma[i] = new_color_vals[9].payload[i];

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d", 
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static ssize_t kgamma_ctrl_store(struct device *dev,
		struct device_attribute *attr, const char *buf, size_t count)
{
	pr_info("kgamma_ctrl count: %d\n", count);
	return count;
}

static ssize_t kgamma_ctrl_show(struct device *dev,
				struct device_attribute *attr, char *buf)
{
	return 0;
}

static DEVICE_ATTR(kgamma_r, 0644, kgamma_r_show, kgamma_r_store);
static DEVICE_ATTR(kgamma_g, 0644, kgamma_g_show, kgamma_g_store);
static DEVICE_ATTR(kgamma_b, 0644, kgamma_b_show, kgamma_b_store);
static DEVICE_ATTR(kgamma_ctrl, 0644, kgamma_ctrl_show, kgamma_ctrl_store);
/******************* end faux123 sysfs interface *******************/

/******************* motley sysfs interface ********************/

/** check for for reasonable values and ones that are too large for the
 * destination char data type
 * */
static bool calc_checksum_generic(unsigned int intArr[]) {
	int i = 0;
	unsigned int chksum = 0;

	if (intArr[5] > 31 || (intArr[6] > 31)) {
		pr_info("gamma 0 and gamma 1 values can't be over 31, got %d %d instead!", intArr[5], intArr[6]);
		return false;
	}

	for (i=1; i<10; i++) {
		if (intArr[i] > 255) {
			pr_info("char values  can't be over 255, got %d instead!", intArr[i]);
			return false;
		}
		chksum += intArr[i];
	}

	if (chksum == intArr[0]) {
		return true;
	} else {
		pr_info("expecting %d, got %d instead!", chksum, intArr[0]);
		return false;
	}
}

static ssize_t kgamma_red_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	unsigned int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum_generic(kgamma)) {
		kgamma[0] = 0xd0;
		for (i=0; i<10; i++) {
			pr_info("kgamma_r_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[5].payload[i] = (char)kgamma[i];
		}

		kgamma[0] = 0xd1;
		for (i=0; i<10; i++) {
			pr_info("kgamma_r_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[6].payload[i] = (char)kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_red_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	unsigned int kgamma[10];
	int i;
	unsigned int check_sum =0;

	for (i=1; i<10; i++) {
		kgamma[i] = (unsigned int)new_color_vals[5].payload[i];
		check_sum += kgamma[i];
	}

	kgamma[0] = check_sum;

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d",
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static ssize_t kgamma_green_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	unsigned int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum_generic(kgamma)) {
		kgamma[0] = 0xd2;
		for (i=0; i<10; i++) {
			pr_info("kgamma_g_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[7].payload[i] = (char)kgamma[i];
		}

		kgamma[0] = 0xd3;
		for (i=0; i<10; i++) {
			pr_info("kgamma_g_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[8].payload[i] = (char)kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_green_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	unsigned int kgamma[10];
	int i;
	unsigned int check_sum =0;

	for (i=1; i<10; i++) {
		kgamma[i] = (unsigned int)new_color_vals[7].payload[i];
		check_sum += kgamma[i];
	}

	kgamma[0] = check_sum;

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d",
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static ssize_t kgamma_blue_store(struct device *dev, struct device_attribute *attr,
						const char *buf, size_t count)
{
	unsigned int kgamma[10];
	int i;

	sscanf(buf, "%d %d %d %d %d %d %d %d %d %d",
		&kgamma[0], &kgamma[1], &kgamma[2], &kgamma[3],
		&kgamma[4], &kgamma[5], &kgamma[6], &kgamma[7],
		&kgamma[8], &kgamma[9]);

	if (calc_checksum_generic(kgamma)) {
		kgamma[0] = 0xd4;
		for (i=0; i<10; i++) {
			pr_info("kgamma_b_p [%d] => %d \n", i, kgamma[i]);
			new_color_vals[9].payload[i] = (char)kgamma[i];
		}

		kgamma[0] = 0xd5;
		for (i=0; i<10; i++) {
			pr_info("kgamma_b_n [%d] => %d \n", i, kgamma[i]);
			new_color_vals[10].payload[i] = (char)kgamma[i];
		}
	}
	return count;
}

static ssize_t kgamma_blue_show(struct device *dev, struct device_attribute *attr,
								char *buf)
{
	unsigned int kgamma[10];
	int i;
	unsigned int check_sum =0;

	for (i=1; i<10; i++) {
		kgamma[i] = (unsigned int)new_color_vals[9].payload[i];
		check_sum += kgamma[i];
	}

	kgamma[0] = check_sum;

	return sprintf(buf, "%d %d %d %d %d %d %d %d %d %d",
		kgamma[0], kgamma[1], kgamma[2], kgamma[3],
		kgamma[4], kgamma[5], kgamma[6], kgamma[7],
		kgamma[8], kgamma[9]);
}

static DEVICE_ATTR(kgamma_red, 0644, kgamma_red_show, kgamma_red_store);
static DEVICE_ATTR(kgamma_green, 0644, kgamma_green_show, kgamma_green_store);
static DEVICE_ATTR(kgamma_blue, 0644, kgamma_blue_show, kgamma_blue_store);

/******************* end motley sysfs interface ********************/

struct syscore_ops panel_syscore_ops = {
	.shutdown = mipi_lgit_lcd_shutdown,
};

static int mipi_lgit_lcd_probe(struct platform_device *pdev)
{
	int rc;

	if (pdev->id == 0) {
		mipi_lgit_pdata = pdev->dev.platform_data;
		return 0;
	}

	// make a copy of platform data
	memcpy((void*)new_color_vals, (void*)mipi_lgit_pdata->power_on_set_1,
		sizeof(new_color_vals));

	pr_info("%s start\n", __func__);

	skip_init = true;
	msm_fb_add_device(pdev);

	register_syscore_ops(&panel_syscore_ops);
	
	/* faux123 gamma control */
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_r);
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_g);
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_b);
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_ctrl);

	/* motley interface */
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_red);
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_green);
	if(rc !=0)
		return -1;
	rc = device_create_file(&pdev->dev, &dev_attr_kgamma_blue);
	if(rc !=0)
		return -1;

	return 0;
}

static struct platform_driver this_driver = {
	.probe = mipi_lgit_lcd_probe,
	.driver = {
		.name = "mipi_lgit",
	},
};

static struct msm_fb_panel_data lgit_panel_data = {
	.on = mipi_lgit_lcd_on,
	.off = mipi_lgit_lcd_off,
	.set_backlight = mipi_lgit_set_backlight_board,
	.get_backlight_on_status = mipi_lgit_backlight_on_status,
};

static int ch_used[3];

int mipi_lgit_device_register(struct msm_panel_info *pinfo,
		u32 channel, u32 panel)
{
	struct platform_device *pdev = NULL;
	int ret;

	if ((channel >= 3) || ch_used[channel])
		return -ENODEV;

	ch_used[channel] = TRUE;

	pdev = platform_device_alloc("mipi_lgit", (panel << 8)|channel);
	if (!pdev)
		return -ENOMEM;

	lgit_panel_data.panel_info = *pinfo;

	ret = platform_device_add_data(pdev, &lgit_panel_data,
			sizeof(lgit_panel_data));
	if (ret) {
		pr_err("%s: platform_device_add_data failed!\n", __func__);
		goto err_device_put;
	}

	ret = platform_device_add(pdev);
	if (ret) {
		pr_err("%s: platform_device_register failed!\n", __func__);
		goto err_device_put;
	}
	return 0;

err_device_put:
	platform_device_put(pdev);
	return ret;
}

static int __init mipi_lgit_lcd_init(void)
{
	mipi_dsi_buf_alloc(&lgit_tx_buf, DSI_BUF_SIZE);
	mipi_dsi_buf_alloc(&lgit_rx_buf, DSI_BUF_SIZE);

	return platform_driver_register(&this_driver);
}

module_init(mipi_lgit_lcd_init);
