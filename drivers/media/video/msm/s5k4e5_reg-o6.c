/* Copyright (c) 2011, Code Aurora Forum. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 and
 * only version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */


#include "s5k4e5-o6.h"

struct s5k4e5_i2c_reg_conf s5k4e5_mipi_settings[] = {
	// MIPI setting				
	{0x30BD, 0x00},		//SEL_CCP[0]				
	{0x3084, 0x15},		//SYNC Mode				
	{0x30BE, 0x1A},		//M_PCLKDIV_AUTO[4], M_DIV_PCLK[3:0]				
	{0x30C1, 0x01},		//pack video enable [0]				
	{0x30EE, 0x02},		//DPHY enable [1]				
	{0x3111, 0x86},		//Embedded data off [5]				
	{0x30E8, 0x0F},		//Continuous mode 			
	{0x30E3, 0x38},		//According to MCLK			
	{0x30E4, 0x40},				
	{0x3113, 0x70},				
	{0x3114, 0x80},				
	{0x3115, 0x7B},				
	{0x3116, 0xC0},				
	{0x30EE, 0x12},				
};

/* PLL Configuration */
struct s5k4e5_i2c_reg_conf s5k4e5_pll_preview_settings[] = {
	// PLL setting ...				
	{0x0305, 0x06},				
	{0x0306, 0x00},
	{0x0307, 0x64},				
	{0x30B5, 0x00},
	{0x30E2, 0x02},		//num lanes[1:0] = 2				
	{0x30F1, 0xD0},		// DPHY Band Control			
	{0x30BC, 0xB0},		// [7]bit : DBLR enable, [6]~[3]bit : DBLR Div			
										// DBLR clock = Pll output/DBLR Div = 66.6Mhz			
	{0x30BF, 0xAB},		//outif_enable[7], data_type[5:0](2Bh = bayer 10bit)				
	{0x30C0, 0x80},		//video_offset[7:4]				
	{0x30C8, 0x0C},		//video_data_length 				
	{0x30C9, 0xBC},				
	{0x3112, 0x00},		//gain option sel off			
	{0x3030, 0x07},		//old shut mode			
};

struct s5k4e5_i2c_reg_conf s5k4e5_pll_snap_settings[] = {
	// PLL setting ...				
	{0x0305, 0x06},				
	{0x0306, 0x00},
	{0x0307, 0x64},				
	{0x30B5, 0x00},
	{0x30E2, 0x02},		//num lanes[1:0] = 2				
	{0x30F1, 0xD0},		// DPHY Band Control			
	{0x30BC, 0xB0},		// [7]bit : DBLR enable, [6]~[3]bit : DBLR Div			
										// DBLR clock = Pll output/DBLR Div = 66.6Mhz			
	{0x30BF, 0xAB},		//outif_enable[7], data_type[5:0](2Bh = bayer 10bit)				
	{0x30C0, 0x80},		//video_offset[7:4]				
	{0x30C8, 0x0C},		//video_data_length 				
	{0x30C9, 0xBC},				
	{0x3112, 0x00},		//gain option sel off			
	{0x3030, 0x07},		//old shut mode			
};

struct s5k4e5_i2c_reg_conf s5k4e5_prev_settings[] = {
	// Integration setting ... 				
	{0x0200, 0x03},		// fine integration time				
	{0x0201, 0x5C},				
//	{0x0202, 0x03},		// coarse integration time				
//	{0x0203, 0xd0},				
//	{0x0204, 0x00},		// Analog Gain				
//	{0x0205, 0x20},				
	{0x0340, 0x07},		// Frame Length				
// jykim temp less than 30 fps (IOT) start
//	{0x0341, 0xB4},				 
	{0x0341, 0xDF},				
// end
	{0x0342, 0x0A},		//line_length_pclk			
	{0x0343, 0xB2},				
	//Size Setting ...				
	// 2608x1960				
	{0x30A9, 0x03},		//Horizontal Binning Off				
	{0x300E, 0x28},		//Vertical Binning Off				
	{0x302B, 0x01},				
	{0x3029, 0x34},		// DBLR & PLA				
	{0x0380, 0x00},		//x_even_inc 1				
	{0x0381, 0x01},
	{0x0382, 0x00},		//x_odd_inc 1
	{0x0383, 0x01},
	{0x0384, 0x00},		//y_even_inc 1
	{0x0385, 0x01},
	{0x0386, 0x00},		//y_odd_inc 3
	{0x0387, 0x01},
	{0x0344, 0x00},		//x_addr_start
	{0x0345, 0x00},
	{0x0346, 0x00},		//y_addr_start
	{0x0347, 0x00},
	{0x0348, 0x0A},		//x_addr_end
	{0x0349, 0x2F},
	{0x034A, 0x07},		//y_addr_end
	{0x034B, 0xA7},
	{0x034C, 0x0A},		//x_output_size_High
	{0x034D, 0x30},
	{0x034E, 0x07},		//y_output_size_High
	{0x034F, 0xA8},
};

struct s5k4e5_i2c_reg_conf s5k4e5_snap_settings[] = {
	// Integration setting ... 				
	{0x0200, 0x03},		// fine integration time				
	{0x0201, 0x5C},				
//	{0x0202, 0x03},		// coarse integration time				
//	{0x0203, 0xd0},				
//	{0x0204, 0x00},		// Analog Gain				
//	{0x0205, 0x20},				
	{0x0340, 0x07},		// Frame Length				
// jykim temp less than 30 fps (IOT) start
//	{0x0341, 0xB4},				 
	{0x0341, 0xDF},				
// end
	{0x0342, 0x0A},		//line_length_pclk			
	{0x0343, 0xB2},
	//Size Setting ...				
	// 2608x1960				
	{0x30A9, 0x03},		//Horizontal Binning Off				
	{0x300E, 0x28},		//Vertical Binning Off				
	{0x302B, 0x01},				
	{0x3029, 0x34},		// DBLR & PLA				
	{0x0380, 0x00},		//x_even_inc 1				
	{0x0381, 0x01},
	{0x0382, 0x00},		//x_odd_inc 1
	{0x0383, 0x01},
	{0x0384, 0x00},		//y_even_inc 1
	{0x0385, 0x01},
	{0x0386, 0x00},		//y_odd_inc 3
	{0x0387, 0x01},
	{0x0344, 0x00},		//x_addr_start
	{0x0345, 0x00},
	{0x0346, 0x00},		//y_addr_start
	{0x0347, 0x00},
	{0x0348, 0x0A},		//x_addr_end
	{0x0349, 0x2F},
	{0x034A, 0x07},		//y_addr_end
	{0x034B, 0xA7},
	{0x034C, 0x0A},		//x_output_size_High
	{0x034D, 0x30},
	{0x034E, 0x07},		//y_output_size_High
	{0x034F, 0xA8},
};

struct s5k4e5_i2c_reg_conf s5k4e5_recommend_settings[] = {
	//+++++++++++++++++++++++++++++++//
	// Analog Setting
	//// CDS timing setting ...
	{0x3000, 0x05},
	{0x3001, 0x03},
	{0x3002, 0x08},
	{0x3003, 0x0A},
	{0x3004, 0x50},
	{0x3005, 0x0E},
	{0x3006, 0x5E},
	{0x3007, 0x00},
	{0x3008, 0x78},
	{0x3009, 0x78},
	{0x300A, 0x50},
	{0x300B, 0x08},
	{0x300C, 0x14},
	{0x300D, 0x00},
	{0x300F, 0x40},
	{0x301B, 0x77},

	// CDS option setting ...
	{0x3010, 0x00},
	{0x3011, 0x3A},
	{0x3012, 0x30},
	{0x3013, 0xA0},
	{0x3014, 0x00},
	{0x3015, 0x00},
	{0x3016, 0x52},
	{0x3017, 0x94},
	{0x3018, 0x70},
	{0x301D, 0xD4},
	{0x3021, 0x02},
	{0x3022, 0x24},
	{0x3024, 0x40},
	{0x3027, 0x08},

	// Pixel option setting ...      
	{0x301C, 0x06},
	{0x30D8, 0x3F},

	//+++++++++++++++++++++++++++++++//
	// ADLC setting ...
	{0x3070, 0x5F},
	{0x3071, 0x00},
	{0x3080, 0x04},
	{0x3081, 0x38},
	{0x302E, 0x0B},

	//+++++++++++++++++++++++++++++++++//
	//Mirror setting
	{0x0101, 0x00},

};

struct s5k4e5_reg s5k4e5_regs = {
	.reg_mipi = &s5k4e5_mipi_settings[0],
	.reg_mipi_size = ARRAY_SIZE(s5k4e5_mipi_settings),
	.rec_settings = &s5k4e5_recommend_settings[0],
	.rec_size = ARRAY_SIZE(s5k4e5_recommend_settings),
	.reg_pll_p = &s5k4e5_pll_preview_settings[0],
	.reg_pll_p_size = ARRAY_SIZE(s5k4e5_pll_preview_settings),
	.reg_pll_s = &s5k4e5_pll_snap_settings[0],
	.reg_pll_s_size = ARRAY_SIZE(s5k4e5_pll_snap_settings),
	.reg_prev = &s5k4e5_prev_settings[0],
	.reg_prev_size = ARRAY_SIZE(s5k4e5_prev_settings),
	.reg_snap = &s5k4e5_snap_settings[0],
	.reg_snap_size = ARRAY_SIZE(s5k4e5_snap_settings),
};
