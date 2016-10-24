/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/
#ifndef BUILD_LK
#include <linux/string.h>
#include <linux/kernel.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
	#include <string.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH                                         (720)
#define FRAME_HEIGHT                                        (1280)
#define LCM_ID                       (0x1283)

#define REGFLAG_DELAY               (0XFE)
#define REGFLAG_END_OF_TABLE        (0x100) // END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE                                    0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)                                    (lcm_util.set_reset_pin((v)))

#define UDELAY(n)                                           (lcm_util.udelay(n))
#define MDELAY(n)                                           (lcm_util.mdelay(n))

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)    lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)       lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)                                      lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)                  lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)                                           lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)               lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	{0x00,1,{0x00}},
	{0xff,3,{0x12,0x83,0x01}},       //EXTC

	{0x00,1,{0x80}},                   //Orise mode enable 
	{0xff,2,{0x12,0x83}},

	//-------------------- power setting -----------------------------------// 
	{0x00,1,{0xc6}},               //debounce 
	{0xB0,1,{0x03}},

	{0x00,1,{0xB9}},               //MIPI driving capability 
	{0xB0,1,{0x51}},

	{0x00,1,{0x80}},            //TCON Setting 
	{0xc0,9,{0x00,0x64,0x00,0x0f,0x11,0x00,0x64,0x0f,0x11}},

	{0x00,1,{0x90}},            //Panel Timing Setting 
	{0xc0,6,{0x00,0x55,0x00,0x01,0x00,0x04}},  //Charge time setting,gate and source 

	{0x00,1,{0xa4}},            //source pre. 
	{0xc0,1,{0x00}},

	{0x00,1,{0xb3}},            //Interval Scan Frame: 0 frame, column inversion 
	{0xc0,2,{0x00,0x50}},   // 50 column, 00 1dot 

	{0x00,1,{0x81}},            //frame rate:60Hz 
	{0xc1,1,{0x55}},

	{0x00,1,{0x80}},            //source to ground
	{0xc4,1,{0x30}},

	{0x00,1,{0x81}},            //source bias 0.75uA 
	{0xc4,1,{0x82}},

	{0x00,1,{0x82}},            //flash-orise add 
	{0xc4,1,{0x02}},

	{0x00,1,{0x87}},            //source precharge to ground
	{0xc4,1,{0x18}},

	{0x00,1,{0x8A}},            //source to ground
	{0xc4,1,{0x40}},

	{0x00,1,{0x8b}},            //source to ground
	{0xc4,1,{0x40}},

	{0x00,1,{0x90}},            //clock delay for data latch 
	{0xc4,1,{0x49}},

	{0x00,1,{0xa0}},            //dcdc setting {PFM Fre) 
	{0xc4,14,{0x05,0x10,0x06,0x02,0x05,0x15,0x10,0x05,0x10,0x07,0x02,0x05,0x15,0x10}},

	{0x00,1,{0xb0}},            //clamp voltage setting 
	{0xc4,2,{0x00,0x00}},        //VSP and VSN Change {5.6V,-5.6V) 

	{0x00,1,{0x90}},            //3xVPNL 
	{0xc5,1,{0x50}},

	{0x00,1,{0x91}},            //VGH=12V, VGL=-12V, pump ratio:VGH=6x, VGL=-5x 
	{0xc5,2,{0x19,0x50}},

	{0x00,1,{0x94}},            //2xVPNL 
	{0xc5,1,{0x66}},

	{0x00,1,{0xb0}},           
	{0xc5,2,{0x04,0x38}},    //VDD_18V=1.7V, LVDSVDD=1.55V  0xb8

	{0x00,1,{0xb4}},            //VGLO1/2 Pull low setting 
	{0xc5,1,{0xc0}},               //d[7] vglo1 d[6] vglo2 => 0: pull vss, 1: pull vgl 

	{0x00,1,{0xbb}},            //LVD voltage level setting 
	{0xc5,1,{0x80}},

	{0x00,1,{0xb5}},
	{0xc5,6,{0x3b,0xed,0x30,0x3b,0xed,0x30}},

	{0x00,1,{0xb1}},            //LED PWM fre.
	{0xc6,1,{0x05}},

	//-------------------------------------- Power IC----------------------------------------- 
	{0x00,1,{0x90}},            //Mode-3 
	{0xf5,4,{0x02,0x11,0x02,0x11}},

	{0x00,1,{0xb2}},            //VGLO1 
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0xb4}},            //VGLO1_S 
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0xb6}},            //VGLO2 
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0xb8}},            //VGLO2_S 
	{0xf5,2,{0x00,0x00}},

	{0x00,1,{0x94}},                 //VCL on           
	{0xF5,1,{0x02}},

	{0x00,1,{0xBA}},                 //VSP on           
	{0xF5,1,{0x03}},

	{0x00,1,{0xC3}},                 //GVDD OP          
	{0xF5,1,{0x81}},

	//-------------------- control setting ---------------------------------------------------// 
	{0x00,1,{0x00}},            //ID1 
	{0xd0,1,{0x40}},

	{0x00,1,{0x00}},            //ID2, ID3 
	{0xd1,2,{0x00,0x00}},

	//-------------------- panel timing state control --------------------//
	{0x00,1,{0x80}},            //panel timing state control
	{0xcb,11,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},

	{0x00,1,{0x90}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xff,0x00,0xff,0x00}},
		   
	{0x00,1,{0xa0}},            //panel timing state control
	{0xcb,15,{0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
		   
	{0x00,1,{0xb0}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0xff,0x00,0x00,0x00,0x00,0x00}},
		   
	{0x00,1,{0xc0}},            //panel timing state control
	{0xcb,15,{0x00,0x00,0x00,0x00,0x05,0x05,0x00,0x00,0x05,0x00,0x05,0x05,0x00,0x05,0x05}},
		   
	{0x00,1,{0xd0}},            //panel timing state control
	{0xcb,15,{0x05,0x05,0x05,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x05,0x05,0x00,0x00}},
		   
	{0x00,1,{0xe0}},            //panel timing state control
	{0xcb,14,{0x05,0x00,0x05,0x05,0x00,0x05,0x05,0x05,0x05,0x05,0x00,0x00,0x00,0x00}},
		   
	{0x00,1,{0xf0}},            //panel timing state control
	{0xcb,11,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},

	//-------------------- panel pad mapping control --------------------//
	 
	{0x00,1,{0x80}},            //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x2E,0x2D,0x00,0x00,0x29,0x00,0x2A,0x0c,0x00,0x0a,0x10}},
		   
	{0x00,1,{0x90}},            //panel pad mapping control
	{0xcc,15,{0x0e,0x02,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2E,0x2D,0x00,0x00}},
		   
	{0x00,1,{0xa0}},            //panel pad mapping control
	{0xcc,14,{0x29,0x00,0x2A,0x0b,0x00,0x09,0x0f,0x0d,0x01,0x03,0x00,0x00,0x00,0x00}},
		   
	{0x00,1,{0xb0}},            //panel pad mapping control
	{0xcc,15,{0x00,0x00,0x00,0x00,0x2D,0x2E,0x00,0x00,0x29,0x00,0x2A,0x0f,0x00,0x09,0x0b}},
		   
	{0x00,1,{0xc0}},            //panel pad mapping control
	{0xcc,15,{0x0d,0x03,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x2D,0x2E,0x00,0x00}},
		   
	{0x00,1,{0xd0}},            //panel pad mapping control
	{0xcc,14,{0x29,0x00,0x2A,0x10,0x00,0x0a,0x0c,0x0e,0x04,0x02,0x00,0x00,0x00,0x00}},

	//-------------------- panel timing setting --------------------//

	{0x00,1,{0x80}},            //panel VST setting
	{0xce,12,{0x8b,0x03,0x10,0x8a,0x03,0x10,0x89,0x03,0x10,0x88,0x03,0x10}},
		   
	{0x00,1,{0x90}},            //panel VEND setting
	{0xce,14,{0xf0,0x00,0x00,0xf0,0x00,0x00,0xf0,0x00,0x00,0xf0,0x00,0x00,0x00,0x00}},
		   
	{0x00,1,{0xa0}},            //panel CLKA1/2 setting
	{0xce,14,{0x38,0x07,0x84,0xfe,0x00,0x10,0x00,0x38,0x06,0x84,0xff,0x00,0x10,0x00}},
		   
	{0x00,1,{0xb0}},            //panel CLKA3/4 setting
	{0xce,14,{0x38,0x05,0x85,0x00,0x00,0x10,0x00,0x38,0x04,0x85,0x01,0x00,0x10,0x00}},
		   
	{0x00,1,{0xc0}},            //panel CLKb1/2 setting
	{0xce,14,{0x38,0x03,0x85,0x02,0x00,0x10,0x00,0x38,0x02,0x85,0x03,0x00,0x10,0x00}},
		   
	{0x00,1,{0xd0}},            //panel CLKb3/4 setting
	{0xce,14,{0x38,0x01,0x85,0x04,0x00,0x10,0x00,0x38,0x00,0x85,0x05,0x00,0x10,0x00}},
		   
	{0x00,1,{0x80}},            //panel CLKc1/2 setting
	{0xcf,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}},
		   
	{0x00,1,{0x90}},            //panel CLKc3/4 setting
	{0xcf,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}},
		   
	{0x00,1,{0xa0}},            //panel CLKd1/2 setting
	{0xcf,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}},
		   
	{0x00,1,{0xb0}},            //panel CLKd3/4 setting
	{0xcf,14,{0x70,0x00,0x00,0x10,0x00,0x00,0x00,0x70,0x00,0x00,0x10,0x00,0x00,0x00}},
		   
	{0x00,1,{0xc0}},            //panel ECLK setting
	{0xcf,11,{0x04,0x01,0x20,0x20,0x00,0x00,0x01,0x00,0x00,0x03,0x08}},//gate pre. ena.
		   
	{0x00,1,{0x00}},            //GVDD=4.87V, NGVDD=-4.87V 
	{0xd8,2,{0x9d,0x9d}},//bc bc

	{0x00,1,{0x00}},          
	{0xd9,1,{0x98}},//83  99

	{0x00,1,{0x00}},
	{0xE1,16,{0x04,0x0E,0x13,0x0E,0x07,0x0D,0x09,0x08,0x06,0x09,0x0F,0x07,0x0E,0x0F,0x0B,0x08}},
	{0x00,1,{0x00}},
	{0xE2,16,{0x04,0x0F,0x14,0x0E,0x07,0x0E,0x09,0x08,0x06,0x09,0x0F,0x07,0x0E,0x0F,0x0B,0x08}},
	//pian bai  gamma
	//{0x00,1,{0x00}},
	//{0xE1,16,{0x04, 0x0a, 0x0E, 0x0D, 0x05, 0x0e, 0x0A, 0x09, 0x04, 0x07, 0x0D, 0x07, 0x0E, 0x16, 0x10, 0x0A}},
	//{0x00,1,{0x00}},
	//{0xE2,16,{0x04, 0x0a, 0x0F, 0x0D, 0x05, 0x0e, 0x0A, 0x09, 0x04, 0x07, 0x0D, 0x07, 0x0E, 0x16, 0x10, 0x0A}},
	  
	{0x00,1,{0x00}},            //Orise mode disable 
	{0xff,3,{0xff,0xff,0xff}},

    {0x11,1,{0x00}}, 
    {REGFLAG_DELAY,120,{}},

    {0x29,1,{0x00}},//Display ON
    {REGFLAG_DELAY,20,{}},   

// Setting ending by predefined flag
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    // Display ON
    //{0x2C, 1, {0x00}},
    //{0x13, 1, {0x00}},
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
    // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},

    // Sleep Mode On
    {0x10, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
    // Display off sequence
    {0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
    {REGFLAG_DELAY, 10, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
    {0x51, 1, {0xFF}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
    unsigned int i;

    for(i = 0; i < count; i++) {
        
        unsigned cmd;
        cmd = table[i].cmd;
        
        switch (cmd) {
            
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
                
            case REGFLAG_END_OF_TABLE :
                break;
                
            default:
                dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
        }
    }
    
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
    memset(params, 0, sizeof(LCM_PARAMS));
    params->type   = LCM_TYPE_DSI;
    params->width  = FRAME_WIDTH;
    params->height = FRAME_HEIGHT;

    // enable tearing-free
    params->dbi.te_mode             = LCM_DBI_TE_MODE_DISABLED;
    params->dbi.te_edge_polarity        = LCM_POLARITY_RISING;



    params->dsi.mode   = SYNC_EVENT_VDO_MODE;


    // DSI
    /* Command mode setting */
    params->dsi.LANE_NUM                = LCM_FOUR_LANE;
    //The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
    params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;
    // Highly depends on LCD driver capability.
    // Not support in MT6573
    params->dsi.packet_size=256;
    // Video mode setting       
    params->dsi.intermediat_buffer_num = 2;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    params->dsi.vertical_sync_active                = 4;
    params->dsi.vertical_backporch                  = 16;
    params->dsi.vertical_frontporch                 = 15;
    params->dsi.vertical_active_line                = FRAME_HEIGHT;
    params->dsi.horizontal_sync_active              = 8;
    params->dsi.horizontal_backporch                = 32;
    params->dsi.horizontal_frontporch               = 32;
    params->dsi.horizontal_blanking_pixel              = 60;
    params->dsi.horizontal_active_pixel            = FRAME_WIDTH;
    // Bit rate calculation

    params->dsi.PLL_CLOCK=220;

	params->dsi.esd_check_enable = 1;
	params->dsi.customization_esd_check_enable = 1;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(20);
    SET_RESET_PIN(1);
    MDELAY(120);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
#ifdef BUILD_LK
    printf("[erick-lk]%s\n", __func__);
#else
    printk("[erick-k]%s\n", __func__);
#endif
}


static void lcm_suspend(void)
{
#ifndef BUILD_LK
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);   //wqtao. enable
    #ifdef BUILD_LK
        printf("[erick-lk]%s\n", __func__);
    #else
        printk("[erick-k]%s\n", __func__);
    #endif
#endif
}


static void lcm_resume(void)
{
#ifndef BUILD_LK
    lcm_init();
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    #ifdef BUILD_LK
        printf("[erick-lk]%s\n", __func__);
    #else
        printk("[erick-k]%s\n", __func__);
    #endif
#endif
}

#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
    unsigned int x0 = x;
    unsigned int y0 = y;
    unsigned int x1 = x0 + width - 1;
    unsigned int y1 = y0 + height - 1;

    unsigned char x0_MSB = ((x0>>8)&0xFF);
    unsigned char x0_LSB = (x0&0xFF);
    unsigned char x1_MSB = ((x1>>8)&0xFF);
    unsigned char x1_LSB = (x1&0xFF);
    unsigned char y0_MSB = ((y0>>8)&0xFF);
    unsigned char y0_LSB = (y0&0xFF);
    unsigned char y1_MSB = ((y1>>8)&0xFF);
    unsigned char y1_LSB = (y1&0xFF);

    unsigned int data_array[16];


    data_array[0]= 0x00053902;
    data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
    data_array[2]= (x1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00053902;
    data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
    data_array[2]= (y1_LSB);
    dsi_set_cmdq(data_array, 3, 1);

    data_array[0]= 0x00290508; //HW bug, so need send one HS packet
    dsi_set_cmdq(data_array, 1, 1);

    data_array[0]= 0x002c3909;
    dsi_set_cmdq(data_array, 1, 0);
}
#endif

#if 0   //wqtao.        
static void lcm_setbacklight(unsigned int level)
{
    unsigned int default_level = 145;
    unsigned int mapped_level = 0;

    //for LGE backlight IC mapping table
    if(level > 255)
            level = 255;

    if(level >0)
            mapped_level = default_level+(level)*(255-default_level)/(255);
    else
            mapped_level=0;

    // Refresh value of backlight level.
    lcm_backlight_level_setting[0].para_list[0] = mapped_level;

    push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
#endif

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_LK
    unsigned char buffer[6];
    unsigned int data_array[16];

    if(lcm_esd_test)
    {
        lcm_esd_test = FALSE;
        return TRUE;
    }

    data_array[0] = 0x00023700;
    dsi_set_cmdq(data_array, 1, 1);
    read_reg_v2(0x0a, buffer, 2);
    printk("[%s] hct_sh1282_dsi_vdo_fwvga_boe esd check: id = %x\n", __FUNCTION__, buffer[0]);

    if(buffer[0] == 0x9c)
    {
        return FALSE;
    }
    else
    {            
        return TRUE;
    }
#endif
}

static unsigned int lcm_esd_recover(void)
{
#ifndef BUILD_LK    
    lcm_init();
    return TRUE;
#endif
}


static unsigned int lcm_compare_id(void)
{
    unsigned int id = 0;
    unsigned char buffer[5];
    unsigned int array[16];

    SET_RESET_PIN(1);   //NOTE:should reset LCM firstly
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(50);

    array[0] = 0x00053700;    // read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0xa1, buffer, 5);
    id = ((buffer[2] << 8) | buffer[3]);    //we only need ID

#ifdef BUILD_LK
    printf("[erick-lk]%s,  otm1283a+cmo id = 0x%08x\n", __func__, id);
#else
    printk("[erick-k]%s,  otm1283a+cmo id = 0x%08x\n", __func__, id);
#endif
    return (LCM_ID == id)?1:0;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_otm1283a_dsi_vdo_hd_lg =
{
    .name            = "hct_otm1283a_dsi_vdo_hd_lg",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,   
    .compare_id    = lcm_compare_id,    
#if (LCM_DSI_CMD_MODE)
    //.set_backlight    = lcm_setbacklight,
    .update         = lcm_update,
#endif  //wqtao
};


