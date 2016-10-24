#include <cust_leds.h>
#include <cust_leds_def.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_gpio.h>
#include <linux/delay.h>
#include "cust_gpio_usage.h"

//extern int mtkfb_set_backlight_level(unsigned int level);
//extern int mtkfb_set_backlight_pwm(int div);
extern int disp_bls_set_backlight(unsigned int level);

// Only support 64 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT 64 
// Support 256 levels of backlight (when lcd-backlight = MT65XX_LED_MODE_PWM)
#define BACKLIGHT_LEVEL_PWM_256_SUPPORT 256 

// Configure the support type "BACKLIGHT_LEVEL_PWM_256_SUPPORT" or "BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT" !!
#define BACKLIGHT_LEVEL_PWM_MODE_CONFIG BACKLIGHT_LEVEL_PWM_256_SUPPORT

unsigned int Cust_GetBacklightLevelSupport_byPWM(void)
{
	return BACKLIGHT_LEVEL_PWM_MODE_CONFIG;
}

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;
}
/*

 * To explain How to set these para for cust_led_list[] of led/backlight
 * "name" para: led or backlight
 * "mode" para:which mode for led/backlight
 *	such as:
 *			MT65XX_LED_MODE_NONE,	
 *			MT65XX_LED_MODE_PWM,	
 *			MT65XX_LED_MODE_GPIO,	
 *			MT65XX_LED_MODE_PMIC,	
 *			MT65XX_LED_MODE_CUST_LCM,	
 *			MT65XX_LED_MODE_CUST_BLS_PWM
 *
 *"data" para: control methord for led/backlight
 *   such as:
 *			MT65XX_LED_PMIC_LCD_ISINK=0,	
 *			MT65XX_LED_PMIC_NLED_ISINK0,
 *			MT65XX_LED_PMIC_NLED_ISINK1,
 *			MT65XX_LED_PMIC_NLED_ISINK2,
 *			MT65XX_LED_PMIC_NLED_ISINK3
 * 
 *"PWM_config" para:PWM(AP side Or BLS module), by default setting{0,0,0,0,0} Or {0}
 *struct PWM_config {	 
 *  int clock_source;
 *  int div; 
 *  int low_duration;
 *  int High_duration;
 *  BOOL pmic_pad;//AP side PWM pin in PMIC chip (only 89 needs confirm); 1:yes 0:no(default)
 *};
 *-------------------------------------------------------------------------------------------
 *   for AP PWM setting as follow:
 *1.	 PWM config data
 *  clock_source: clock source frequency, can be 0/1
 *  div: clock division, can be any value within 0~7 (i.e. 1/2^(div) = /1, /2, /4, /8, /16, /32, /64, /128)
 *  low_duration: only for BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT
 *  High_duration: only for BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT
 *
 *2.	 PWM freq.
 * If BACKLIGHT_LEVEL_PWM_MODE_CONFIG = BACKLIGHT_LEVEL_PWM_256_SUPPORT,
 *	 PWM freq. = clock source / 2^(div) / 256  
 *
 * If BACKLIGHT_LEVEL_PWM_MODE_CONFIG = BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT,
 *	 PWM freq. = clock source / 2^(div) / [(High_duration+1)(Level')+(low_duration+1)(64 - Level')]
 *	           = clock source / 2^(div) / [(High_duration+1)*64]     (when low_duration = High_duration)
 *Clock source: 
 *	 0: block clock/1625 = 26M/1625 = 16K (MT6571)
 *	 1: block clock = 26M (MT6571)
 *Div: 0~7
 *
 *For example, in MT6571, PWM_config = {1,1,0,0,0} 
 *	 ==> PWM freq. = 26M/2^1/256 	 =	50.78 KHz ( when BACKLIGHT_LEVEL_PWM_256_SUPPORT )
 *	 ==> PWM freq. = 26M/2^1/(0+1)*64 = 203.13 KHz ( when BACKLIGHT_LEVEL_PWM_64_FIFO_MODE_SUPPORT )
 *-------------------------------------------------------------------------------------------
 *   for BLS PWM setting as follow:
 *1.	 PWM config data
 *	 clock_source: clock source frequency, can be 0/1/2/3
 *	 div: clock division, can be any value within 0~1023
 *	 low_duration: non-use
 *	 High_duration: non-use
 *	 pmic_pad: non-use
 *
 *2.	 PWM freq.= clock source / (div + 1) /1024
 *Clock source: 
 *	 0: 26 MHz
 *	 1: 104 MHz
 *	 2: 124.8 MHz
 *	 3: 156 MHz
 *Div: 0~1023
 *
 *By default, clock_source = 0 and div = 0 => PWM freq. = 26 KHz 
 *-------------------------------------------------------------------------------------------
 */


#ifdef GPIO_RGB_RED_PIN
int rgb_set_red_by_gpio(unsigned int level)
{
	if(level > 0)
	{
		mt_set_gpio_mode(GPIO_RGB_RED_PIN, GPIO_RGB_RED_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_RED_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_RED_PIN,GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_RGB_RED_PIN, GPIO_RGB_RED_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_RED_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_RED_PIN,GPIO_OUT_ZERO);
	}
	
}
#endif

#ifdef GPIO_RGB_GREEN_PIN
int rgb_set_green_by_gpio(unsigned int level)
{
	if(level > 0)
	{
		mt_set_gpio_mode(GPIO_RGB_GREEN_PIN, GPIO_RGB_GREEN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_GREEN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_GREEN_PIN,GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_RGB_GREEN_PIN, GPIO_RGB_GREEN_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_GREEN_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_GREEN_PIN,GPIO_OUT_ZERO);
	}
	
}
#endif

#ifdef GPIO_RGB_BLUE_PIN
int rgb_set_blue_by_gpio(unsigned int level)
{
	if(level > 0)
	{
		mt_set_gpio_mode(GPIO_RGB_BLUE_PIN, GPIO_RGB_BLUE_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_BLUE_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_BLUE_PIN,GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_RGB_BLUE_PIN, GPIO_RGB_BLUE_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_RGB_BLUE_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_RGB_BLUE_PIN,GPIO_OUT_ZERO);
	}
	
}
#endif

#ifdef GPIO_BUTTON_LED_PIN
int rgb_set_Button_led_by_gpio(unsigned int level)
{
	if(level > 0)
	{
		mt_set_gpio_mode(GPIO_BUTTON_LED_PIN, GPIO_BUTTON_LED_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_BUTTON_LED_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_BUTTON_LED_PIN,GPIO_OUT_ONE);
	}
	else
	{
		mt_set_gpio_mode(GPIO_BUTTON_LED_PIN, GPIO_BUTTON_LED_PIN_M_GPIO);
		mt_set_gpio_dir(GPIO_BUTTON_LED_PIN, GPIO_DIR_OUT);
		mt_set_gpio_out(GPIO_BUTTON_LED_PIN,GPIO_OUT_ZERO);
	}
	
}
#endif


#ifdef GPIO_BL_LDO_EN_PIN  // this is plus count method

static DEFINE_SPINLOCK(bl_lock);

volatile kal_uint8 g_ledCurrPulseCount=1;//mut out of this func ,or

void brightness_set_gpio(unsigned int level) 
{ 
        int i; 
        int min_level = 30;        //the min value for UI brightness setup 
        int max_level = 255;        //the max value for UI brightness setup 
        int min_bri = 1;        //the min value for adjust the brightness 
        int max_bri = 32;        //the max value for adjust the brightness 
        kal_uint8 count=0, to_count=0; 
        static int s_bBacklightOn = 1; 
        struct mtk_irq_mask mask;
        mt_set_gpio_mode(GPIO_BL_LDO_EN_PIN, GPIO_MODE_GPIO); 
        mt_set_gpio_dir(GPIO_BL_LDO_EN_PIN, GPIO_DIR_OUT); 
	//	printk("hsl: brightness_set_gpio level=%d\n",level);
        if (level) 
		{ 
             if (level < min_level) 
		    	{  
		    		to_count = max_bri; 
				}
		   	 else if (level > 255)
		     	{ 
		     		to_count = min_bri;   
				}
		     else 
		    	{ 
                    to_count = max_bri-((level-min_level)*(max_bri-min_bri)/(max_level-min_level)); 
                } 
			 
	            if (s_bBacklightOn) 
				{ 
					if (g_ledCurrPulseCount > to_count) 
					{   
						count = to_count+32-g_ledCurrPulseCount;  
					}//change brighter  
					else if (g_ledCurrPulseCount < to_count)
					{   
						count = to_count-g_ledCurrPulseCount;   
					}   //change darker 
					else 
					{  	
						goto end;      
					} 
				}
			   else
			   { 
			   		count = to_count;    
			   } 
		   
				if (s_bBacklightOn)
				{ 
				        spin_lock(&bl_lock);
				        for (i=0; i<count; i++)
						{ 
				                mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ZERO); 
				                udelay(10); 
				                mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ONE); 
				                udelay(10);         
				        } 
				        spin_unlock(&bl_lock);				
				}
				else
				{        //first pulse must set high more than 500 us 
				        spin_lock(&bl_lock);
				        mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ONE); 
				        udelay(500); 

				        for (i=1; i<count; i++) 
						{ 
				                mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ZERO); 
				                udelay(10); 
				                mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ONE); 
				                udelay(10);                             
				        } 
				        spin_unlock(&bl_lock);

				} 
                g_ledCurrPulseCount = to_count; 
                udelay(3); 
                s_bBacklightOn = 1; 
        }
		else 
		{ 
                mt_set_gpio_out(GPIO_BL_LDO_EN_PIN, GPIO_OUT_ZERO); 
                s_bBacklightOn = 0; 
                udelay(3); 
        } 
end:
	return ;
} 
unsigned int lcd_brightness_mapping(unsigned int level)
{
	return level;
}
#endif
static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
#if defined(CONFIG_T89_PROJ)||defined(CONFIG_T985_PROJ)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_GPIO, (long)rgb_set_green_by_gpio,{0}},
	{"blue",              MT65XX_LED_MODE_GPIO, (long)rgb_set_blue_by_gpio,{0}},
#elif defined(CONFIG_T875_PROJ)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_GPIO, (long)rgb_set_green_by_gpio,{0}},
	{"blue",              MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
#elif defined(CONFIG_T925_PROJ)
    #if defined(CONFIG_T925T_DG_PROJ)// no RGB leds
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
    #else
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_GPIO, (long)rgb_set_green_by_gpio,{0}},
	    #if defined(CONFIG_T925_TIEPIAN)
		{"blue",              MT65XX_LED_MODE_GPIO, (long)rgb_set_blue_by_gpio,{0}},
	    #elif defined(CONFIG_T925D_NYX_M18) // used for sub flashlight
		{"blue",             MT65XX_LED_MODE_NONE, -1,{0}},
	    #else
		{"blue",              MT65XX_LED_MODE_GPIO, (long)rgb_set_blue_by_gpio,{0}},
	    #endif
    #endif
#elif defined(CONFIG_T93_PROJ)
#if defined(CONFIG_RGB_LED_SUPPORT)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
	{"blue",              MT65XX_LED_MODE_GPIO, (long)rgb_set_blue_by_gpio,{0}},
#else
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
#endif
#elif defined(CONFIG_T96_PROJ)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
#elif defined(CONFIG_T99_PROJ)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_GPIO, (long)rgb_set_green_by_gpio,{0}},
	{"blue",              MT65XX_LED_MODE_GPIO, (long)rgb_set_blue_by_gpio,{0}},
#elif defined(CONFIG_T823_PROJ)
	{"red",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK1,{0}},
	{"green",             MT65XX_LED_MODE_GPIO, (long)rgb_set_green_by_gpio,{0}},
	{"blue",               MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0}},
#else
	{"red",               MT65XX_LED_MODE_NONE, -1,{0}},
	{"green",             MT65XX_LED_MODE_NONE, -1,{0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1,{0}},
#endif
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1,{0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1,{0}},
#if defined(CONFIG_T93_PROJ) || defined(CONFIG_T875_PROJ)
		{"button-backlight",	MT65XX_LED_MODE_GPIO, (long)rgb_set_Button_led_by_gpio,{0}},
#elif defined(CONFIG_T823_PROJ)
		{"button-backlight",	MT65XX_LED_MODE_NONE, -1,{0}},
#else
		{"button-backlight",  MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_NLED_ISINK0,{0,0,0,0,0}},
#endif
#if defined(GPIO_BL_LDO_EN_PIN)
		{"lcd-backlight",	  MT65XX_LED_MODE_CUST_LCM, brightness_set_gpio},
#else
	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (long)disp_bls_set_backlight,{0}},
#endif
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

