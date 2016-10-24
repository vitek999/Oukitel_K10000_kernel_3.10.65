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

#define FRAME_WIDTH                                         (1080)
#define FRAME_HEIGHT                                        (1920)
#define LCM_ID                       (0x3315)

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
{0xB0,1,{0x04}},	//Manufacturer Command

{0xD6,1,{0x01}},	//Sequencer Test Control

{0xB3,6,{0x14,0x00,0x00,0x00,0x00,0x00}},	//interface setting video mode

{0xB4,2,{0x0C,0x00}},	//interface ID setting DSI_4Lane

{0xB6,2,{0x3A,0xD3}},	//DSI_control

{0xB7,1,{0x00}},	//Checksum and ECC error count

{0xB8,6,{0x07,0x90,0x1E,0x10,0x1E,0x32}},	//Back light control 1

{0xB9,6,{0x07,0x82,0x3C,0x10,0x3C,0x87}},	//Back light control 2

{0xBA,6,{0x07,0x78,0x64,0x10,0x64,0xB4}},	//Back light control 3

{0xBE,2,{0x00,0x04}},	//External clock setting

{0xC0,1,{0x11}},	//SOUT Slew rate adjustment

{0xC1,34,{0x84,0x60,0x00,0xFF,0x8F,
          0xF2,0xD1,0x31,0xE1,0x47,
          0xF8,0x5C,0x63,0xAC,0xB9,
          0x07,0xE3,0x07,0xE6,0xFC,
          0x4F,0xC4,0xFF,0x00,0x00,
          0x00,0x00,0x00,0x68,0x01,
          0x00,0x22,0x00,0x01}},	//Display setting 1

{0xC2,7,{0x31,0xF7, 0x80,0x06,0x06,0x00,0x00}},	//Sub pix inversion,VPB,VFP

{0xC3,3,{0x00,0x00,0x00}},	//tpc_sync_control vsout hsout off

{0xC4,22,{0x70,0x01,0x00,0x01,0x01,
          0x01,0x01,0x01,0x01,0x01,
          0x03,0x00,0x00,0x00,0x00,
          0x00,0x00,0x00,0x00,0x00,
          0x01,0x03}},	//Sorce timing setting

{0xC5,1,{0x00}},	//Real timing scaling off

{0xC6,40,{0x71,0x00,0x00,0x08,0x66,
          0x00,0x00,0x00,0x00,0x00,
          0x00,0x00,0x00,0x00,0x00,
          0x00,0x00,0x14,0x16,0x07,
          0x71,0x00,0x00,0x08,0x66,
          0x00,0x00,0x00,0x00,0x00,
          0x00,0x00,0x00,0x00,0x00,
          0x00,0x00,0x14,0x16,0x07}},	//ST_SW

{0xC7,30,{0x00,0x13,0x1B,0x25,0x32,
          0x40,0x4A,0x5A,0x3F,0x47,
          0x53,0x60,0x69,0x6F,0x74,
          0x00,0x13,0x1B,0x25,0x32,
          0x40,0x4A,0x5A,0x3F,0x47,
          0x53,0x60,0x69,0x6F,0x74}},	//Analog Gamma setting

{0xC8,19,{0x00,0x00,0xFC,0x01,0x00,
          0xFC,0x00,0x00,0xFD,0x02,
          0x03,0xDE,0x00,0x00,0xFA,
          0x03,0x08,0xBA,0x00}},	//Digtal Gamma setting

{0xCB,9,{0xEC,0xFD,0xBF,0x37,0x20,0x00,0x00,0x04,0xC0}},	//Panel pin control

{0xCC,1,{0x0E}},	//Panel interface control typeB

{0xCF,5,{0x00,0x00,0xC1,0x05,0x3F}},	//GPO control off

{0xD0,10,{0x11,0x85,0xBB,0x5D,0xCF,0x4C,0x19,0x19,0x0C,0x00}},	//Power setting 1 for charge pump

//Power setting 2
{0xD1,26,{0x20,0x00,0x00,0x04,0x08,
          0x0C,0x10,0x00,0x00,0x00,
          0x00,0x00,0x3C,0x04,0x20,
          0x00,0x00,0x04,0x08,0x0C,
          0x10,0x00,0x00,0x3C,0xC6,
          0xF0}},	

//Power setting for internal power
{0xD3,25,{0x1B,0x33,0xBB,0xBB,0xB3,
          0x33,0x33,0x33,0x00,0x01,
          0x00,0xA0,0xE8,0xA0,0x0D,
          0x46,0x46,0x33,0x3B,0x37,
          0x72,0x57,0x3D,0xBF,0x33}},

//VCOM setting	
{0xD5,7,{0x06,0x00,0x00,0x01,0x52,0x01,0x52}},	

//Sequence timing control for power on
{0xD7,22,{0x84,0xE0,0x7F,0xA8,0xCE,
          0x38,0xFC,0xC1,0x18,0xE7,
          0x8F,0x1F,0x3C,0x10,0xFA,
          0xC3,0x0F,0x04,0x41,0x00,
          0x00,0x00}},

{0xDD,2,{0x10,0x8C}},	//Outline sharpening control

//Test image generator
{0xDE,4,{0x00,0xFF,0x07,0x10}},	

{0x3A,1,{0x70}},	//set pixel format

	{0x29,1,{0x00}},//SLEEP OUT
	{REGFLAG_DELAY,20,{}},
																								
	{0x11,1,{0x00}},//Display ON 
	{REGFLAG_DELAY,120,{}},

// Setting ending by predefined flag
    {REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {

      // Sleep Out
    {0x29, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    // Display ON
    //{0x2C, 1, {0x00}},
    //{0x13, 1, {0x00}},
    {0x11, 1, {0x00}},
    {REGFLAG_DELAY, 120, {}},
    {REGFLAG_END_OF_TABLE, 0x00, {}}
 

};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {

        // Display off sequence
    {0x28, 1, {0x00}},
    {REGFLAG_DELAY, 20, {}},
    
    {0xD3,25,{0x13,0x33,0xBB,0xB3,0xB3,0x33,0x33,0x33,0x00,0x01,0x00,0xA0,0xE8,0xA0,0x0D,0x46,0x46,0x33,0x3B,0x37,0x72,0x57,0x3D,0xBF,0x33}},
    {REGFLAG_DELAY, 120, {}},
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

    params->dsi.mode   = BURST_VDO_MODE;

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
    params->dsi.intermediat_buffer_num = 0;
    params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    params->dsi.vertical_sync_active                = 2;//2;//
    params->dsi.vertical_backporch                  = 4;//38;//
    params->dsi.vertical_frontporch                 = 8;//40;//
    params->dsi.vertical_active_line                = FRAME_HEIGHT; 
    params->dsi.horizontal_sync_active              = 12;//24;//
    params->dsi.horizontal_backporch                =40;//75;//
    params->dsi.horizontal_frontporch               = 96;//75;//
 //   params->dsi.horizontal_blanking_pixel              = 60;
    params->dsi.horizontal_active_pixel            = FRAME_WIDTH;
    // Bit rate calculation

    params->dsi.PLL_CLOCK=430;//230
   // params->dsi.ssc_disable=1;

	params->dsi.esd_check_enable = 0;
	params->dsi.customization_esd_check_enable = 0;
	params->dsi.lcm_esd_check_table[0].cmd = 0x0a;
	params->dsi.lcm_esd_check_table[0].count = 1;
	params->dsi.lcm_esd_check_table[0].para_list[0] = 0x9c;
}

static void lcm_init(void)
{
    SET_RESET_PIN(1);  //shm
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);

    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
   SET_RESET_PIN(1);  //shm
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);
    push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);   //wqtao. enable
}


static void lcm_resume(void)
{
     push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    //push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
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


static unsigned int lcm_compare_id(void)
{
    unsigned int id = 0;
    unsigned char buffer[5];
    unsigned int array[16];

    SET_RESET_PIN(1);  //shm
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(20);

    array[0] = 0x00053700;  // read id return two byte,version and id
    dsi_set_cmdq(array, 1, 1);
    read_reg_v2(0xbf, buffer, 5);
     MDELAY(20);
    id = ((buffer[2] << 8) | buffer[3]);    //we only need ID

#ifdef BUILD_LK
	printf("%s,  otm1287a+tm id = 0x%08x\n", __func__, id);
#else
	printk("%s,  otm1287a+tm id = 0x%08x\n", __func__, id);
#endif
    return (LCM_ID == id)?1:0;
   //return 1;
}

// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER hct_r63315_dsi_vdo_fhd_tm = 
{
    .name           = "hct_r63315_dsi_vdo_fhd_tm",
    .set_util_funcs = lcm_set_util_funcs,
    .get_params     = lcm_get_params,
    .init           = lcm_init,
    .suspend        = lcm_suspend,
    .resume         = lcm_resume,   
    .compare_id    = lcm_compare_id,    
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};

