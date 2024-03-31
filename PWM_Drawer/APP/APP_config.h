/*
 * APP_config.h
 */

#ifndef APP_CONFIG_H_
#define APP_CONFIG_H_

#include "STM32F103.h"

/*Timers configurations-----------------------------------------------------*/
#define APP_TIM_PWM_TIMx (TIM2)
#define APP_TIM_PWM_CHx (TIM_CH3)

#define APP_TIM_IC1_TIMx (TIM3)
#define APP_TIM_IC1_CHx (TIM_CH1)

#define APP_TIM_IC2_TIMx (TIM1)
#define APP_TIM_IC2_CHx (TIM_CH2)

#define APP_GLCD_LOW_LINE (7UL)
#define APP_GLCD_HIGH_LINE (5UL)

/*GLCD configurations-----------------------------------------------------*/
#define APP_GLCD_CYCLE_1_START_X (7UL)
#define APP_GLCD_CYCLE_2_START_X (APP_GLCD_CYCLE_1_START_X + 60UL)
#define APP_GLCD_CYCLE_2_END_X (127UL)

#endif /* APP_CONFIG_H_ */