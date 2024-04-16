/*
 * APP_program.c
 */

/* Includes ------------------------------------------------------------------*/
#include "BIT_MATH.h"

#include "../MCAL/TIM/TIM_interface.h"
#include "../HAL/GLCD/GLCD_interface.h"
#include "../MCAl/GPIO/GPIO_interface.h"

#include "APP_interface.h"
#include "APP_config.h"

/* Private functions prototypes ----------------------------------------------*/
static void APP_IC_Calculate_Freq_Duty(void);
static void APP_GLCD_DrawPWM(u32 duty);
static f32 APP_IC_GetFreq_KHZ();
static f32 APP_IC_GetPeriod_ms();
static u32 APP_IC_GetDuty();
static void APP_GLCD_PrintFreq();
static void APP_GLCD_PrintDuty();
static void APP_GLCD_PrintPeriod();

/* Private variables --------------------------------------------------------*/
static u8 IsFirstCapture = 0;

static u32 firstRisingEdgeCount = 0;
static u32 secondRisingEdgeCount = 0;
static u32 risingEdgeDifference = 0;

static u32 dutydiff = 0;

static f32 frequency = 0;
static u32 duty = 0;

static f32 oldFreq = 0;
static f32 oldDuty = 0;
static f32 oldPeriod =0;

/* Private functions --------------------------------------------------------*/
static void APP_IC_Calculate_Freq_Duty(void)
{
	u32 TIMx_CCRx = 0;

	switch (APP_TIM_IC1_CHx)
	{
	case TIM_CH1:
		TIMx_CCRx = APP_TIM_IC1_TIMx->CCR1;
		break;

	case TIM_CH2:
		TIMx_CCRx = APP_TIM_IC1_TIMx->CCR2;
		break;

	case TIM_CH3:
		TIMx_CCRx = APP_TIM_IC1_TIMx->CCR3;
		break;

	case TIM_CH4:
		TIMx_CCRx = APP_TIM_IC1_TIMx->CCR4;
		break;
	}

	if (IsFirstCapture == 0)
	{
		firstRisingEdgeCount = TIMx_CCRx;
		IsFirstCapture = 1;
	}
	else
	{
		secondRisingEdgeCount = TIMx_CCRx;

		if (secondRisingEdgeCount > firstRisingEdgeCount)
		{
			risingEdgeDifference = secondRisingEdgeCount - firstRisingEdgeCount;
		}
		else
		{
			risingEdgeDifference = (TIM_MAX_PERIOD - firstRisingEdgeCount) + secondRisingEdgeCount;
		}

		if (APP_TIM_IC2_TIMx->CCR2 > firstRisingEdgeCount)
		{
			dutydiff = APP_TIM_IC2_TIMx->CCR2 - firstRisingEdgeCount;
		}
		else
		{
			dutydiff = (TIM_MAX_PERIOD - firstRisingEdgeCount) + APP_TIM_IC2_TIMx->CCR2;
		}

		if (risingEdgeDifference > 1)
		{
			frequency = (float)TIM_CLK / (float)risingEdgeDifference;
			duty = (dutydiff * 100 ) / (float)risingEdgeDifference;
			IsFirstCapture = 0;
		}
	}
}


static void APP_GLCD_DrawPWM(u32 duty)
{
	for (int i=APP_GLCD_HIGH_LINE ; i<=APP_GLCD_LOW_LINE ; ++i)
	{
		GLCD_ClearLine(i);
	}
	int dutyXrange = (duty * 60) / 100;

	GLCD_DrawHLine(0, APP_GLCD_CYCLE_1_START_X, APP_GLCD_LOW_LINE);

	// Drawing the first cycle
	GLCD_DrawVLine(APP_GLCD_HIGH_LINE, APP_GLCD_LOW_LINE, APP_GLCD_CYCLE_1_START_X);
	GLCD_DrawHLine(APP_GLCD_CYCLE_1_START_X + 1, (APP_GLCD_CYCLE_1_START_X + dutyXrange), APP_GLCD_HIGH_LINE);
	GLCD_DrawVLine(APP_GLCD_HIGH_LINE, APP_GLCD_LOW_LINE, APP_GLCD_CYCLE_1_START_X + dutyXrange);
	GLCD_DrawHLine(APP_GLCD_CYCLE_1_START_X + dutyXrange, APP_GLCD_CYCLE_2_START_X, APP_GLCD_LOW_LINE);

	// Drawing the second cycle
	GLCD_DrawVLine(APP_GLCD_HIGH_LINE, APP_GLCD_LOW_LINE, APP_GLCD_CYCLE_2_START_X);
	GLCD_DrawHLine(APP_GLCD_CYCLE_2_START_X + 1, (APP_GLCD_CYCLE_2_START_X + dutyXrange), APP_GLCD_HIGH_LINE);
	GLCD_DrawVLine(APP_GLCD_HIGH_LINE, APP_GLCD_LOW_LINE, APP_GLCD_CYCLE_2_START_X + dutyXrange);
	GLCD_DrawHLine(APP_GLCD_CYCLE_2_START_X + dutyXrange, APP_GLCD_CYCLE_2_END_X, APP_GLCD_LOW_LINE);
}

static void APP_GLCD_PrintFreq()
{
	GLCD_ClearLine(APP_GLCD_FREQ_LINE);
	f32 glcdFreq = APP_IC_GetFreq_KHZ();
	GLCD_PrintString("FREQ:", 0, APP_GLCD_FREQ_LINE);
	GLCD_PrintFloat(glcdFreq, 35, APP_GLCD_FREQ_LINE);
	GLCD_PrintString("KHZ", 90, APP_GLCD_FREQ_LINE);
}

static void APP_GLCD_PrintDuty()
{
	GLCD_ClearLine(APP_GLCD_DUTY_LINE);
	u32 glcdDuty = APP_IC_GetDuty();
	GLCD_PrintString("DUTY:", 0, APP_GLCD_DUTY_LINE);
	GLCD_PrintNum(glcdDuty, 35, APP_GLCD_DUTY_LINE);
	GLCD_PrintString("%", 55, APP_GLCD_DUTY_LINE);
}

static void APP_GLCD_PrintPeriod()
{
	GLCD_ClearLine(APP_GLCD_PERIOD_LINE);
	f32 glcdPeriod = APP_IC_GetPeriod_ms();
	GLCD_PrintString("PERIOD:", 0, APP_GLCD_PERIOD_LINE);
	GLCD_PrintFloat(glcdPeriod, 50, APP_GLCD_PERIOD_LINE);
	GLCD_PrintString("ms", 100, APP_GLCD_PERIOD_LINE);
}
/* Public functions --------------------------------------------------------*/
void APP_Init(void)
{
	GLCD_Init();
	TIM_Init(APP_TIM_IC1_TIMx);

	TIM_Init(APP_TIM_IC2_TIMx);

	TIM_Init(APP_TIM_PWM_TIMx);
}

void APP_IC_Start()
{
	GPIO_SetPinDirSpeed(GPIOA, GPIO_PIN9, GPIO_INPUT_FLOATING);
	GPIO_SetPinDirSpeed(GPIOA, GPIO_PIN6, GPIO_INPUT_FLOATING);

	TIM_IC_Start(APP_TIM_IC1_TIMx, APP_TIM_IC1_CHx, CCS_IP_DIRECT, TIM_IC_RISING_EDGE);


	TIM_IC_INT_Enable(APP_TIM_IC1_TIMx);

	if (APP_TIM_IC1_TIMx == TIM1)
	{
		TIM1_CC_SetCallback(APP_IC_Calculate_Freq_Duty);
	}

	else if (APP_TIM_IC1_TIMx == TIM2)
	{
		TIM2_SetCallback(APP_IC_Calculate_Freq_Duty);
	}
	else if (APP_TIM_IC1_TIMx == TIM3)
	{
		TIM3_SetCallback(APP_IC_Calculate_Freq_Duty);
	}

	TIM_IC_Start(APP_TIM_IC2_TIMx, APP_TIM_IC2_CHx, CCS_IP_DIRECT, TIM_IC_FALLING_EDGE);

}

void APP_PWM_Start(u32 dutyCycle, u32 frequency)
{

	TIM_PWM_Start(APP_TIM_PWM_TIMx, APP_TIM_PWM_CHx, dutyCycle, frequency);
}

static f32 APP_IC_GetFreq_KHZ()
{
	// return frequency in KHZ
	return frequency / 1000;
}

f32 APP_IC_GetPeriod_ms()
{
	 //return period in ms
	int temp = frequency;
	if (frequency == 0)
		temp =1;

	return ((1.0 / (temp)) * 1000);
}

u32 APP_IC_GetDuty()
{
	return duty;
}

void APP_GLCD_Print_Init()
{
	oldFreq = APP_IC_GetFreq_KHZ();
	oldDuty = APP_IC_GetDuty();
	oldPeriod = APP_IC_GetPeriod_ms();

	APP_GLCD_PrintFreq();
	APP_GLCD_PrintDuty();
	APP_GLCD_PrintPeriod();
	APP_GLCD_DrawPWM(duty);

}
void APP_GLCD_Update()
{
	if (APP_IC_GetFreq_KHZ() != oldFreq)
	{
		oldFreq = APP_IC_GetFreq_KHZ();
		APP_GLCD_PrintFreq();
	}

	if (APP_IC_GetDuty() != oldDuty)
	{
		oldDuty = APP_IC_GetDuty();
		APP_GLCD_PrintDuty();

	}
	if (APP_IC_GetPeriod_ms() != oldPeriod)
	{
		oldPeriod = APP_IC_GetPeriod_ms();
		APP_GLCD_PrintPeriod();
	}

}
