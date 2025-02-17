/**
  ******************************************************************************
  * @file    pwmc_3pwm.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides the firmware functions that implement the 
  *          pwmc_3pwm component of the Motor Control SDK.
  *
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  * @ingroup pwmc_3pwm
  */

/* Includes ------------------------------------------------------------------*/
#include "pwmc_3pwm.h"
#include "pwm_common_sixstep.h"
#include "mc_type.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup pwm_curr_fdbk_6s
  * @{
  */

/**
 * @defgroup pwmc_3pwm Six-Step, 3 signals PWM generation
 *
 * @brief PWM generation implementation for Six-Step drive with 3 PWM duty cycles
 *
 * This implementation only drives the high sides of the bridges. The low sides are 
 * automatically driven by the gate driver that inserts the dead time. 
 * 
 * Three additional signals are generated to enable or disable the PWM output of each
 * phase.
 *
 * @todo: Complete documentation.
 * @{
 */

/* Private defines -----------------------------------------------------------*/
#define TIMxCCER_MASK_CH123       (LL_TIM_CHANNEL_CH1 | LL_TIM_CHANNEL_CH2 | LL_TIM_CHANNEL_CH3)
								   
/* Private function prototypes -----------------------------------------------*/
void ThreePwm_ResetEnablePins( PWMC_ThreePwm_Handle_t * pHandle );

/**
  * @brief  It initializes TIMx and NVIC
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void ThreePwm_Init( PWMC_ThreePwm_Handle_t * pHandle )
{
  if ( ( uint32_t )pHandle == ( uint32_t )&pHandle->_Super )
  {

    /* Enable the CCS */
    LL_RCC_HSE_EnableCSS();

    /* Peripheral clocks enabling END ----------------------------------------*/

    /* Clear TIMx break flag. */
    LL_TIM_ClearFlag_BRK( pHandle->pParams_str->TIMx );
    LL_TIM_EnableIT_BRK( pHandle->pParams_str->TIMx );
    LL_TIM_ClearFlag_UPDATE( pHandle->pParams_str->TIMx );
    
//    /* TIMx Counter Clock stopped when the core is halted */
//    if ( pHandle->pParams_str->TIMx == TIM1 )
//   {
//      /* TIM1 Counter Clock stopped when the core is halted */
//      LL_DBGMCU_APB2_GRP1_FreezePeriph( LL_DBGMCU_APB2_GRP1_TIM1_STOP );
//    }
//    else
//    {
//      /* TIM8 Counter Clock stopped when the core is halted */
//      LL_DBGMCU_APB2_GRP1_FreezePeriph( LL_DBGMCU_APB2_GRP1_TIM8_STOP );
//    }

    /* disable ADC source trigger */
    LL_TIM_SetTriggerOutput(pHandle->pParams_str->TIMx, LL_TIM_TRGO_RESET);
	
    /* Enable PWM channel */
    LL_TIM_CC_EnableChannel( pHandle->pParams_str->TIMx, TIMxCCER_MASK_CH123 );

	/* Clear the flags */
    pHandle->OverVoltageFlag = false;
    pHandle->OverCurrentFlag = false;

    pHandle->_Super.hElAngle = 0;
    LL_TIM_EnableCounter( pHandle->pParams_str->TIMx );
  }
}

/**
* @brief  It updates the stored duty cycle variable.
* @param  pHandle Pointer on the target component instance.
* @param  new duty cycle value.
* @retval none
*/
__weak void PWMC_SetPhaseVoltage( PWMC_Handle_t * pHandle, uint16_t DutyCycle )
{
  pHandle->CntPh = DutyCycle;
}

/**
* @brief  It writes the duty cycle into timer shadow registers.
* @param  pHandle Pointer on the target component instance.
* @retval none
*/
__weak void ThreePwm_LoadNextStep( PWMC_ThreePwm_Handle_t * pHandle )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;
  /* Quasi synchronous rectification */
  pHandle->_Super.NextStep = PWMC_ElAngleToStep(&(pHandle->_Super));

  /* Fast demagnetization */
  if ( pHandle->FastDemag )
  {
    switch ( pHandle->_Super.NextStep )
	{
      case STEP_1:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
      }
        break;
      case STEP_2:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
      case STEP_3:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
      case STEP_4:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
      }
        break;
      case STEP_5:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        }
        break;
      case STEP_6:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
    }	
  }
  /* Alignment between two adjacent steps, CW direction*/
  else if ( pHandle->_Super.AlignFlag == 1 )
  {
    switch ( pHandle->_Super.NextStep )
    {
      case STEP_1:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );  
      }
        break;
      case STEP_2:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                 
      }
        break;
      case STEP_3:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_4:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_5:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_6:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
    }
  }
  /* Alignment between two adjacent steps, CCW direction*/
  else if ( pHandle->_Super.AlignFlag == -1 )
  {
    switch ( pHandle->_Super.NextStep )
    {
      case STEP_1:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                 
      }
        break;
      case STEP_2:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_3:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_4:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_5:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );		
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );                                  
      }
        break;
      case STEP_6:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );  
      }
        break;
    }
  }
	/* standard configuration */
  else 
  {
    switch ( pHandle->_Super.NextStep )
    {
      case STEP_1:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
      }
        break;
      case STEP_2:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
      case STEP_3:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
      case STEP_4:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_TIM_OC_SetCompareCH3 ( TIMx, 0 );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
      }
        break;
      case STEP_5:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
      case STEP_6:
      {
        LL_TIM_OC_SetCompareCH1 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH2 ( TIMx, 0 );
        LL_TIM_OC_SetCompareCH3 ( TIMx, (uint32_t)pHandle->_Super.CntPh );
        LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
        LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
      }
        break;
    }			
  }
}

/**
* @brief  It uploads the values into registers.
* @param  pHandle Pointer on the target component instance.
* @retval bool Registers have been updated
*/
__weak bool ThreePwm_ApplyNextStep( PWMC_ThreePwm_Handle_t * pHandle )
{
  bool retVal = false;
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;
  if (pHandle->_Super.Step != pHandle->_Super.NextStep)
  {
    LL_TIM_GenerateEvent_COM( TIMx );
    pHandle->_Super.Step = pHandle->_Super.NextStep;
    retVal = true;
    LL_TIM_SetCounter( TIMx, (pHandle->_Super.PWMperiod - LL_TIM_GetCounter(TIMx)) );  
  }
  return retVal;
}

/**
* @brief  It toggles the polarity of the timer PWM channel outputs.
* @param  pHandle Pointer on the target component instance.
* @retval none
*/
__weak void ThreePwm_SetOCOppositePolarity( PWMC_ThreePwm_Handle_t * pHandle )
{
  uint32_t PWM_ch_polarity;
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;
  if (pHandle->pParams_str->OCPolarity == LL_TIM_OCPOLARITY_LOW) PWM_ch_polarity = LL_TIM_OCPOLARITY_HIGH;
  else PWM_ch_polarity = LL_TIM_OCPOLARITY_LOW;
  LL_TIM_OC_SetPolarity( TIMx, TIMxCCER_MASK_CH123, PWM_ch_polarity);
}

/**
* @brief  It resets the polarity of the timer PWM channel outputs to default.
* @param  pHandle Pointer on the target component instance.
* @retval none
*/
__weak void ThreePwm_ResetOCPolarity( PWMC_ThreePwm_Handle_t * pHandle )
{
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;
  LL_TIM_OC_SetPolarity( TIMx, TIMxCCER_MASK_CH123, pHandle->pParams_str->OCPolarity);
}

/**
  * @brief  It turns on low sides switches. This function is intended to be
  *         used for charging boot capacitors of driving section. It has to be
  *         called each motor start-up when using high voltage drivers
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void ThreePwm_TurnOnLowSides( PWMC_Handle_t * pHdl )
{
#if defined (__ICCARM__)
#pragma cstat_disable = "MISRAC2012-Rule-11.3"
#endif
  PWMC_ThreePwm_Handle_t * pHandle = ( PWMC_ThreePwm_Handle_t * )pHdl;
#if defined (__ICCARM__)
#pragma cstat_restore = "MISRAC2012-Rule-11.3"
#endif
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->_Super.TurnOnLowSidesAction = true;

  /* Clear Update Flag */
  LL_TIM_ClearFlag_UPDATE( TIMx );

  /*Turn on the three low side switches */
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);

  /* Main PWM Output Enable */
  LL_TIM_EnableAllOutputs(TIMx);
  
  LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
  LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
  LL_GPIO_SetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );  
}

/**
  * @brief  This function enables the PWM outputs
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void ThreePwm_SwitchOnPWM( PWMC_Handle_t * pHdl )
{
#if defined (__ICCARM__)
#pragma cstat_disable = "MISRAC2012-Rule-11.3"
#endif
  PWMC_ThreePwm_Handle_t * pHandle = ( PWMC_ThreePwm_Handle_t * )pHdl;
#if defined (__ICCARM__)
#pragma cstat_restore = "MISRAC2012-Rule-11.3"
#endif
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->_Super.TurnOnLowSidesAction = false;

  LL_TIM_EnableIT_UPDATE( pHandle->pParams_str->TIMx );
	/* Select the Capture Compare preload feature */
  LL_TIM_CC_EnablePreload( TIMx );
	/* Select the Commutation event source */
  LL_TIM_CC_SetUpdate( TIMx, LL_TIM_CCUPDATESOURCE_COMG_ONLY );
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);
  LL_TIM_OC_SetMode( TIMx, LL_TIM_CHANNEL_CH1, LL_TIM_OCMODE_PWM1 );
  LL_TIM_OC_SetMode( TIMx, LL_TIM_CHANNEL_CH2, LL_TIM_OCMODE_PWM1 );
  LL_TIM_OC_SetMode( TIMx, LL_TIM_CHANNEL_CH3, LL_TIM_OCMODE_PWM1 );		
	/* Main PWM Output Enable */
  TIMx->BDTR |= LL_TIM_OSSI_ENABLE;
  LL_TIM_EnableAllOutputs(TIMx);
  LL_TIM_SetTriggerOutput(TIMx, LL_TIM_TRGO_OC4REF);
}

/**
  * @brief  This function sets the capcture compare of the timer channel 
  * used for ADC triggering
  * @param  pHdl: handler of the current instance of the PWM component
  * @param  SamplingPoint: trigger point
  * @retval none
  */
__weak void ThreePwm_SetADCTriggerChannel( PWMC_Handle_t * pHdl, uint16_t SamplingPoint )
{
#if defined (__ICCARM__)
#pragma cstat_disable = "MISRAC2012-Rule-11.3"
#endif
	PWMC_ThreePwm_Handle_t * pHandle = ( PWMC_ThreePwm_Handle_t * )pHdl;
#if defined (__ICCARM__)
#pragma cstat_restore = "MISRAC2012-Rule-11.3"
#endif
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->_Super.ADCTriggerCnt = SamplingPoint;
  LL_TIM_OC_SetCompareCH4(TIMx, pHandle->_Super.ADCTriggerCnt);
}

/**
  * @brief  It disables PWM generation on the proper Timer peripheral acting on
  *         MOE bit and reset the TIM status
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval none
  */
__weak void ThreePwm_SwitchOffPWM( PWMC_Handle_t * pHdl )
{
#if defined (__ICCARM__)
#pragma cstat_disable = "MISRAC2012-Rule-11.3"
#endif
  PWMC_ThreePwm_Handle_t * pHandle = ( PWMC_ThreePwm_Handle_t * )pHdl;
#if defined (__ICCARM__)
#pragma cstat_restore = "MISRAC2012-Rule-11.3"
#endif
  TIM_TypeDef * TIMx = pHandle->pParams_str->TIMx;

  pHandle->_Super.TurnOnLowSidesAction = false;

  ThreePwm_ResetEnablePins ( pHandle );
  
  /* Main PWM Output Disable */
  LL_TIM_DisableAllOutputs(TIMx);
  LL_TIM_OC_SetCompareCH1(TIMx, 0u);
  LL_TIM_OC_SetCompareCH2(TIMx, 0u);
  LL_TIM_OC_SetCompareCH3(TIMx, 0u);
  pHandle->_Super.CntPh = 0;
  LL_TIM_DisableIT_UPDATE( pHandle->pParams_str->TIMx );
  LL_TIM_SetTriggerOutput(pHandle->pParams_str->TIMx, LL_TIM_TRGO_RESET);
  return;
}

/**
 * @brief  It contains the Break event interrupt
 * @param  pHandle: handler of the current instance of the PWM component
 * @retval none
 */
__weak void * ThreePwm_BRK_IRQHandler( PWMC_ThreePwm_Handle_t * pHandle )
{
  pHandle->OverCurrentFlag = true;

  return MC_NULL;
}

/**
  * @brief  It is used to check if an overcurrent occurred since last call.
  * @param  pHdl: handler of the current instance of the PWM component
  * @retval uint16_t It returns MC_BREAK_IN whether an overcurrent has been
  *                  detected since last method call, MC_NO_FAULTS otherwise.
  */
__weak uint16_t ThreePwm_IsOverCurrentOccurred( PWMC_Handle_t * pHdl )
{
#if defined (__ICCARM__)
#pragma cstat_disable = "MISRAC2012-Rule-11.3"
#endif
  PWMC_ThreePwm_Handle_t * pHandle = ( PWMC_ThreePwm_Handle_t * )pHdl;
#if defined (__ICCARM__)
#pragma cstat_restore = "MISRAC2012-Rule-11.3"
#endif
  uint16_t retVal = MC_NO_FAULTS;


  if ( pHandle->OverVoltageFlag == true )
  {
    retVal = MC_OVER_VOLT;
    pHandle->OverVoltageFlag = false;
  }

  if ( pHandle->OverCurrentFlag == true )
  {
    retVal |= MC_BREAK_IN;
    pHandle->OverCurrentFlag = false;
  }
  return retVal;
}

void ThreePwm_ResetEnablePins( PWMC_ThreePwm_Handle_t * pHandle )
{
  LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_u_port, pHandle->pParams_str->pwm_en_u_pin );
  LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_v_port, pHandle->pParams_str->pwm_en_v_pin );
  LL_GPIO_ResetOutputPin( pHandle->pParams_str->pwm_en_w_port, pHandle->pParams_str->pwm_en_w_pin );
}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT 2022 STMicroelectronics *****END OF FILE****/
