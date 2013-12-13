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
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301, USA.
 *
 */


#include "mt9e013-o6.h"

struct mt9e013_i2c_reg_conf mipi_settings[] = {
	/*Disable embedded data*/
	{0x3064, 0x7800},/*SMIA_TEST*/
	/*configure 2-lane MIPI*/
	{0x31AE, 0x0202},/*SERIAL_FORMAT*/
	{0x31B8, 0x0E3F},/*MIPI_TIMING_2*/
	/*set data to RAW10 format*/
	{0x0112, 0x0A0A},/*CCP_DATA_FORMAT*/
	{0x30F0, 0x8000},/*VCM CONTROL*/
};

/*PLL Configuration
(Ext=24MHz, vt_pix_clk=174MHz, op_pix_clk=69.6MHz)*/
struct mt9e013_i2c_reg_conf pll_settings[] = {
	{0x0300, 0x0004},/*VT_PIX_CLK_DIV*/
	{0x0302, 0x0001},/*VT_SYS_CLK_DIV*/
	{0x0304, 0x0002},/*PRE_PLL_CLK_DIV*/
	{0x0306, 0x003A},/*PLL_MULTIPLIER*/
	{0x0308, 0x000A},/*OP_PIX_CLK_DIV*/
	{0x030A, 0x0001},/*OP_SYS_CLK_DIV*/
};

struct mt9e013_i2c_reg_conf prev_settings[] = {
	/*Output Size (1632x1224)*/
	{0x0344, 0x0018},/*X_ADDR_START*/
//	{0x0348, 0x0CC9},/*X_ADDR_END*/
	{0x0348, 0x0CAF},/*X_ADDR_END*/
	{0x0346, 0x0018},/*Y_ADDR_START*/
//	{0x034A, 0x0999},/*Y_ADDR_END*/
	{0x034A, 0x098F},/*Y_ADDR_END*/
#if 1 // 1632x1224
//	{0x034C, 0x0660},/*X_OUTPUT_SIZE*/
//	{0x034E, 0x04C8},/*Y_OUTPUT_SIZE*/
	{0x034C, 0x064C},/*X_OUTPUT_SIZE*/// 1612
	{0x034E, 0x04BC},/*Y_OUTPUT_SIZE*/// 1212
#else // 1280x960
	{0x034C, 0x0510},/*X_OUTPUT_SIZE*/ // - 1280x960
	{0x034E, 0x03D0},/*Y_OUTPUT_SIZE*/
#endif	
	{0x306E, 0xFCB0},/*DATAPATH_SELECT*/
	{0x3040, 0x14C3},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0002},/*SCALING_MODE*/
#if 1 // 1632x1224
	{0x0404, 0x0010},/*SCALE_M*/
#else // 1280x960
	{0x0404, 0x0014},/*SCALE_M*/
#endif	
	/*Timing configuration*/
	{0x0342, 0x1018},/*LINE_LENGTH_PCK*/
	{0x0340, 0x055B},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x0040},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x0846},/*FINE_INTEGRATION_TIME_*/
	{0x3010, 0x0130},/*FINE_CORRECTION*/
};

struct mt9e013_i2c_reg_conf snap_settings[] = {
	/*Output Size (3264x2448)*/
	{0x0344, 0x001E},/*X_ADDR_START */
//	{0x0348, 0x0CD7},/*X_ADDR_END*/ 
	{0x0348, 0x0CA9},/*X_ADDR_END*/ // fov
	{0x0346, 0x001E},/*Y_ADDR_START */
//	{0x034A, 0x09A7},/*Y_ADDR_END*/
	{0x034A, 0x0989},/*Y_ADDR_END*/ // fov
//	{0x034C, 0x0CD0},/*X_OUTPUT_SIZE*/
//	{0x034E, 0x09A0},/*Y_OUTPUT_SIZE*/
	{0x034C, 0x0C8C},/*X_OUTPUT_SIZE*/ // fov
	{0x034E, 0x096C},/*Y_OUTPUT_SIZE*/ // fov
	{0x306E, 0xFC80},/*DATAPATH_SELECT*/
	{0x3040, 0x0041},/*READ_MODE*/
	{0x3178, 0x0000},/*ANALOG_CONTROL5*/
	{0x3ED0, 0x1E24},/*DAC_LD_4_5*/
	{0x0400, 0x0000},/*SCALING_MODE*/
	{0x0404, 0x0010},/*SCALE_M*/
	/*Timing configuration*/
	{0x0342, 0x1248},/*LINE_LENGTH_PCK*/
	{0x0340, 0x0A1F},/*FRAME_LENGTH_LINES*/
	{0x0202, 0x0A1F},/*COARSE_INTEGRATION_TIME*/
	{0x3014, 0x03F6},/*FINE_INTEGRATION_TIME_ */
	{0x3010, 0x0078},/*FINE_CORRECTION*/
};

struct mt9e013_i2c_reg_conf recommend_settings[] = {
#if 0 // original recommend set	
	{0x3044, 0x0590},
	{0x306E, 0xFC80},
	{0x30B2, 0xC000},
	{0x30D6, 0x0800},
	{0x316C, 0xB42F},
	{0x316E, 0x869C},
	{0x3170, 0x210E},
	{0x317A, 0x010E},
	{0x31E0, 0x1FB9},
	{0x31E6, 0x07FC},
	{0x37C0, 0x0000},
	{0x37C2, 0x0000},
	{0x37C4, 0x0000},
	{0x37C6, 0x0000},
	{0x3E02, 0x8801},
	{0x3E04, 0x2301},
	{0x3E06, 0x8449},
	{0x3E08, 0x6841},
	{0x3E0A, 0x400C},
	{0x3E0C, 0x1001},
	{0x3E0E, 0x2103},
	{0x3E10, 0x4B41},
	{0x3E12, 0x4B26},
	{0x3E16, 0x8802},
	{0x3E18, 0x84FF},
	{0x3E1A, 0x8601},
	{0x3E1C, 0x8401},
	{0x3E1E, 0x840A},
	{0x3E20, 0xFF00},
	{0x3E22, 0x8401},
	{0x3E24, 0x00FF},
	{0x3E26, 0x0088},
	{0x3E28, 0x2E8A},
	{0x3E32, 0x8801},
	{0x3E34, 0x4024},
	{0x3E38, 0x8469},
	{0x3E3C, 0x2301},
	{0x3E3E, 0x3E25},
	{0x3E40, 0x1C01},
	{0x3E42, 0x8486},
	{0x3E44, 0x8401},
	{0x3E46, 0x00FF},
	{0x3E48, 0x8401},
	{0x3E4A, 0x8601},
	{0x3E4C, 0x8402},
	{0x3E4E, 0x00FF},
	{0x3E50, 0x6623},
	{0x3E52, 0x8340},
	{0x3E54, 0x00FF},
	{0x3E56, 0x4A42},
	{0x3E58, 0x2203},
	{0x3E5A, 0x674D},
	{0x3E5C, 0x3F25},
	{0x3E5E, 0x846A},
	{0x3E60, 0x4C01},
	{0x3E62, 0x8401},
	{0x3E66, 0x3901},
	{0x3ECC, 0x00EB},
	{0x3ED0, 0x1E24},
	{0x3ED4, 0xAFC4},
	{0x3ED6, 0x909B},
	{0x3ED8, 0x0006},
	{0x3EDA, 0xCFC6},
	{0x3EDC, 0x4FE4},
	{0x3EE0, 0x2424},
	{0x3EE2, 0x9797},
	{0x3EE4, 0xC100},
	{0x3EE6, 0x0540},
	{0x31B0, 0x0083},	//MIPI_timing
	{0x31B2, 0x004D},
/* move to mode setting - 
	{0x31B4, 0x0E77},
	{0x31B6, 0x0D24},
	{0x31B8, 0x020E},
	{0x31BA, 0x0710},
	{0x31BC, 0x2A0D}
*/
#else // new recommend set
	{0x3044, 0x0590},
	{0x306E, 0xFC80},
	{0x30B2, 0xC000},
	{0x30D6, 0x0800},
	{0x316C, 0xB42F},
	{0x316E, 0x869A},
	{0x3170, 0x210E},
	{0x317A, 0x010E},
	{0x31E0, 0x1FB9},
	{0x31E6, 0x07FC},
	{0x37C0, 0x0000},
	{0x37C2, 0x0000},
	{0x37C4, 0x0000},
	{0x37C6, 0x0000},
	{0x3E00, 0x0011},
	{0x3E02, 0x8801},
	{0x3E04, 0x2801}, 
	{0x3E06, 0x8449},
	{0x3E08, 0x6841},
	{0x3E0A, 0x400C},
	{0x3E0C, 0x1001},
	{0x3E0E, 0x2603}, 
	{0x3E10, 0x4B41},
	{0x3E12, 0x4B24}, 
	{0x3E14, 0xA3CF},
	{0x3E16, 0x8802},
	{0x3E18, 0x84FF},
	{0x3E1A, 0x8601},
	{0x3E1C, 0x8401},
	{0x3E1E, 0x840A},
	{0x3E20, 0xFF00},
	{0x3E22, 0x8401},
	{0x3E24, 0x00FF},
	{0x3E26, 0x0088},
	{0x3E28, 0x2E8A},
	{0x3E30, 0x0000},
	{0x3E32, 0x8801},
	{0x3E34, 0x4029}, 
	{0x3E36, 0x00FF},
	{0x3E38, 0x8469},
	{0x3E3A, 0x00FF},
	{0x3E3C, 0x2801},
	{0x3E3E, 0x3E2A}, 
	{0x3E40, 0x1C01},
	{0x3E42, 0xFF84}, 
	{0x3E44, 0x8401},
	{0x3E46, 0x0C01}, 
	{0x3E48, 0x8401},
	{0x3E4A, 0x00FF}, 
	{0x3E4C, 0x8402},
	{0x3E4E, 0x8984},
	{0x3E50, 0x6628}, 
	{0x3E52, 0x8340},
	{0x3E54, 0x00FF},
	{0x3E56, 0x4A42},
	{0x3E58, 0x2703}, 
	{0x3E5A, 0x6752},
	{0x3E5C, 0x3F2A}, 
	{0x3E5E, 0x846A},
	{0x3E60, 0x4C01},
	{0x3E62, 0x8401},
	{0x3E66, 0x3901},
	{0x3E90, 0x2C01},	
	{0x3E98, 0x2B02},	
	{0x3E92, 0x2A04},	
	{0x3E94, 0x2509},	
	{0x3E96, 0x0000},	
	{0x3E9A, 0x2905},	 
	{0x3E9C, 0x00FF},	
	{0x3ECC, 0x00EB},
	{0x3ED0, 0x1E24},
	{0x3ED4, 0xAFC4},
	{0x3ED6, 0x909B},
	{0x3EE0, 0x2424},
	{0x3EE2, 0x9797},
	{0x3EE4, 0xC100},
	{0x3EE6, 0x0540},
	{0x3174, 0x8000},
	{0x31B0, 0x0083},    	//MIPI_timing
	{0x31B2, 0x004D},    
	{0x31B4, 0x0E77},    
	{0x31B6, 0x0D20},    
	{0x31B8, 0x020E},    
	{0x31BA, 0x0710},    
	{0x31BC, 0x2A0D},
#endif
};

// default sensor lsc table - 2011.03.30
struct mt9e013_i2c_reg_conf lsc_settings[] = {
	{  0x3600, 0x01D0  },  	//  P_GR_P0Q0
	{  0x3602, 0x7E6D  },  	//  P_GR_P0Q1
	{  0x3604, 0x0C30  },  	//  P_GR_P0Q2
	{  0x3606, 0xC52D  },  	//  P_GR_P0Q3
	{  0x3608, 0x8870  },  	//  P_GR_P0Q4
	{  0x360A, 0x02D0  },  	//  P_RD_P0Q0
	{  0x360C, 0xE86D  },  	//  P_RD_P0Q1
	{  0x360E, 0x624F  },  	//  P_RD_P0Q2
	{  0x3610, 0x28CE  },  	//  P_RD_P0Q3
	{  0x3612, 0xC4AF  },  	//  P_RD_P0Q4
	{  0x3614, 0x0350  },  	//  P_BL_P0Q0
	{  0x3616, 0x23EE  },  	//  P_BL_P0Q1
	{  0x3618, 0x236F  },  	//  P_BL_P0Q2
	{  0x361A, 0x9ACE  },  	//  P_BL_P0Q3
	{  0x361C, 0x9FCF  },  	//  P_BL_P0Q4
	{  0x361E, 0x0270  },  	//  P_GB_P0Q0
	{  0x3620, 0x89AE  },  	//  P_GB_P0Q1
	{  0x3622, 0x1450  },  	//  P_GB_P0Q2
	{  0x3624, 0x2A6E  },  	//  P_GB_P0Q3
	{  0x3626, 0x9790  },  	//  P_GB_P0Q4
	{  0x3640, 0x888D  },  	//  P_GR_P1Q0
	{  0x3642, 0xFF6D  },  	//  P_GR_P1Q1
	{  0x3644, 0x7148  },  	//  P_GR_P1Q2
	{  0x3646, 0x0DCE  },  	//  P_GR_P1Q3
	{  0x3648, 0x360C  },  	//  P_GR_P1Q4
	{  0x364A, 0xF6EC  },  	//  P_RD_P1Q0
	{  0x364C, 0x746D  },  	//  P_RD_P1Q1
	{  0x364E, 0x73CC  },  	//  P_RD_P1Q2
	{  0x3650, 0xB3CE  },  	//  P_RD_P1Q3
	{  0x3652, 0xD089  },  	//  P_RD_P1Q4
	{  0x3654, 0x178D  },  	//  P_BL_P1Q0
	{  0x3656, 0x3DAE  },  	//  P_BL_P1Q1
	{  0x3658, 0x2C8E  },  	//  P_BL_P1Q2
	{  0x365A, 0x832F  },  	//  P_BL_P1Q3
	{  0x365C, 0xA24F  },  	//  P_BL_P1Q4
	{  0x365E, 0x200D  },  	//  P_GB_P1Q0
	{  0x3660, 0x904E  },  	//  P_GB_P1Q1
	{  0x3662, 0x170E  },  	//  P_GB_P1Q2
	{  0x3664, 0x49AE  },  	//  P_GB_P1Q3
	{  0x3666, 0x9D2F  },  	//  P_GB_P1Q4
	{  0x3680, 0x4650  },  	//  P_GR_P2Q0
	{  0x3682, 0x752D  },  	//  P_GR_P2Q1
	{  0x3684, 0xAD12  },  	//  P_GR_P2Q2
	{  0x3686, 0xC14E  },  	//  P_GR_P2Q3
	{  0x3688, 0x4852  },  	//  P_GR_P2Q4
	{  0x368A, 0x4B10  },  	//  P_RD_P2Q0
	{  0x368C, 0x04E8  },  	//  P_RD_P2Q1
	{  0x368E, 0x8CF2  },  	//  P_RD_P2Q2
	{  0x3690, 0x854E  },  	//  P_RD_P2Q3
	{  0x3692, 0x2152  },  	//  P_RD_P2Q4
	{  0x3694, 0x3150  },  	//  P_BL_P2Q0
	{  0x3696, 0x580D  },  	//  P_BL_P2Q1
	{  0x3698, 0x91D2  },  	//  P_BL_P2Q2
	{  0x369A, 0x83CE  },  	//  P_BL_P2Q3
	{  0x369C, 0x2D92  },  	//  P_BL_P2Q4
	{  0x369E, 0x4050  },  	//  P_GB_P2Q0
	{  0x36A0, 0x4E8D  },  	//  P_GB_P2Q1
	{  0x36A2, 0xB1F2  },  	//  P_GB_P2Q2
	{  0x36A4, 0x8F0F  },  	//  P_GB_P2Q3
	{  0x36A6, 0x55B2  },  	//  P_GB_P2Q4
	{  0x36C0, 0x6B8D  },  	//  P_GR_P3Q0
	{  0x36C2, 0x498E  },  	//  P_GR_P3Q1
	{  0x36C4, 0x634D  },  	//  P_GR_P3Q2
	{  0x36C6, 0xE4AE  },  	//  P_GR_P3Q3
	{  0x36C8, 0xA6D0  },  	//  P_GR_P3Q4
	{  0x36CA, 0x330E  },  	//  P_RD_P3Q0
	{  0x36CC, 0xB58E  },  	//  P_RD_P3Q1
	{  0x36CE, 0x962E  },  	//  P_RD_P3Q2
	{  0x36D0, 0x566F  },  	//  P_RD_P3Q3
	{  0x36D2, 0xCDAF  },  	//  P_RD_P3Q4
	{  0x36D4, 0xEA6A  },  	//  P_BL_P3Q0
	{  0x36D6, 0xA78F  },  	//  P_BL_P3Q1
	{  0x36D8, 0xCAAF  },  	//  P_BL_P3Q2
	{  0x36DA, 0x1B50  },  	//  P_BL_P3Q3
	{  0x36DC, 0x73EF  },  	//  P_BL_P3Q4
	{  0x36DE, 0x31CA  },  	//  P_GB_P3Q0
	{  0x36E0, 0x496E  },  	//  P_GB_P3Q1
	{  0x36E2, 0xE4EF  },  	//  P_GB_P3Q2
	{  0x36E4, 0x8E2F  },  	//  P_GB_P3Q3
	{  0x36E6, 0x07B0  },  	//  P_GB_P3Q4
	{  0x3700, 0x89F1  },  	//  P_GR_P4Q0
	{  0x3702, 0xC16F  },  	//  P_GR_P4Q1
	{  0x3704, 0x22F2  },  	//  P_GR_P4Q2
	{  0x3706, 0x2FCF  },  	//  P_GR_P4Q3
	{  0x3708, 0x7E10  },  	//  P_GR_P4Q4
	{  0x370A, 0x8671  },  	//  P_RD_P4Q0
	{  0x370C, 0x638E  },  	//  P_RD_P4Q1
	{  0x370E, 0x7871  },  	//  P_RD_P4Q2
	{  0x3710, 0xD630  },  	//  P_RD_P4Q3
	{  0x3712, 0x75F1  },  	//  P_RD_P4Q4
	{  0x3714, 0xFEB0  },  	//  P_BL_P4Q0
	{  0x3716, 0xCA8F  },  	//  P_BL_P4Q1
	{  0x3718, 0x15D2  },  	//  P_BL_P4Q2
	{  0x371A, 0x298F  },  	//  P_BL_P4Q3
	{  0x371C, 0x5690  },  	//  P_BL_P4Q4
	{  0x371E, 0x82F1  },  	//  P_GB_P4Q0
	{  0x3720, 0x68ED  },  	//  P_GB_P4Q1
	{  0x3722, 0x3072  },  	//  P_GB_P4Q2
	{  0x3724, 0x92D0  },  	//  P_GB_P4Q3
	{  0x3726, 0x1690  },  	//  P_GB_P4Q4
	{  0x3782, 0x062C  },  	//  POLY_ORIGIN_C
	{  0x3784, 0x047C  },  	//  POLY_ORIGIN_R
	{  0x37C0, 0x31EA  },  	//  P_GR_Q5
	{  0x37C2, 0x230A  },  	//  P_RD_Q5
	{  0x37C4, 0x076B  },  	//  P_BL_Q5
	{  0x37C6, 0x268A  },  	//  P_GB_Q5
};

struct mt9e013_reg mt9e013_regs = {
	.reg_mipi = &mipi_settings[0],
	.reg_mipi_size = ARRAY_SIZE(mipi_settings),
	.rec_settings = &recommend_settings[0],
	.rec_size = ARRAY_SIZE(recommend_settings),
	.reg_pll = &pll_settings[0],
	.reg_pll_size = ARRAY_SIZE(pll_settings),
	.reg_prev = &prev_settings[0],
	.reg_prev_size = ARRAY_SIZE(prev_settings),
	.reg_snap = &snap_settings[0],
	.reg_snap_size = ARRAY_SIZE(snap_settings),
	.reg_lsc = &lsc_settings[0],
	.reg_lsc_size = ARRAY_SIZE(lsc_settings),
};
