/**
  ******************************************************************************
  * @file    revup_ctrl_sixstep.h
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file contains all definitions and functions prototypes for the
  *          Six-step variant of the Rev up control component of the Motor Control 
  *          SDK.
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
  * @ingroup RevUpCtrl6S
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef REVUP_CTRL_SIXSTEP_H
#define REVUP_CTRL_SIXSTEP_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/* Includes ------------------------------------------------------------------*/
#include "mc_type.h"
#include "speed_ctrl.h"
#include "virtual_speed_sensor.h"
#include "bemf_speed_pos_fdbk.h"
#include "pwm_common_sixstep.h"

/** @addtogroup MCSDK
  * @{
  */

/** @addtogroup RevUpCtrl
  * @{
  */

/** @addtogroup RevUpCtrl6S
 * @{
 */

/* Exported constants --------------------------------------------------------*/
/**
  * @brief Maximum number of phases allowed for RevUp process.
  *
  */
#define RUC_MAX_PHASE_NUMBER 5u

/**
  * @brief RevUpCtrl_PhaseParams_t structure used for phases definition
  *
  */
typedef struct
{
  uint16_t hDurationms;         /**< Duration of the RevUp phase.
                                     This parameter is expressed in millisecond.*/
  int16_t hFinalMecSpeedUnit;   /**< Mechanical speed assumed by VSS at the end of
                                     the RevUp phase. Expressed in the unit defined
                                     by #SPEED_UNIT */
  uint16_t hFinalPulse;         /**< Motor torque reference imposed by STC at the
                                     end of RevUp phase. This value represents
                                     actually the Iq current expressed in digit.*/
  void * pNext;                 /**< Pointer on the next phase section to proceed
                                     This parameter is NULL for the last element.*/
} RevUpCtrl_PhaseParams_t;

/**
* @brief  Handle structure of the RevUpCtrl.
*
*/

typedef struct
{
  uint16_t hRUCFrequencyHz;        /**< Frequency call to main RevUp procedure RUC_Exec.
                                        This parameter is equal to speed loop frequency. */

  int16_t hStartingMecAngle;       /**< Starting angle of programmed RevUp.*/

  uint16_t hPhaseRemainingTicks;   /**< Number of clock events remaining to complete the phase. */

  int16_t hDirection;              /**< Motor direction.
                                        This parameter can be any value -1 or +1 */

  RevUpCtrl_PhaseParams_t *pCurrentPhaseParams; /**< Pointer on the current RevUp phase processed. */

  RevUpCtrl_PhaseParams_t ParamsData[RUC_MAX_PHASE_NUMBER]; /**< Start up Phases sequences used by RevUp controller.
                                                                 Up to five phases can be used for the start up. */

  uint8_t bPhaseNbr;               /**< Number of phases relative to the programmed RevUp sequence.
                                        This parameter can be any value from 1 to 5 */

  uint8_t bFirstAccelerationStage; /**< Indicate the phase to start the final acceleration.
                                        At start of this stage sensor-less algorithm cleared.*/
  uint16_t hMinStartUpValidSpeed;  /**< Minimum rotor speed required to validate the startup.
                                        This parameter is expressed in SPPED_UNIT */
  bool EnteredZone1;              /**< Flag to indicate that the minimum rotor speed has been reached. */

  uint8_t bStageCnt;              /**< Counter of executed phases.
                                      This parameter can be any value from 0 to 5 */

  SpeednTorqCtrl_Handle_t *pSTC;          /**< Speed and torque controller object used by RevUpCtrl.*/

  VirtualSpeedSensor_Handle_t *pVSS;      /**< Virtual speed sensor object used by RevUpCtrl.*/

  Bemf_Handle_t *pSNSL;                    /**< STO sensor object used by OTF startup.*/

  PWMC_Handle_t *pPWM;                    /**< PWM object used by OTF startup.*/

} RevUpCtrl_Handle_t;


/* Exported functions ------------------------------------------------------- */

/*  Function used to initialize and configure the RevUpCtrl Component */
void RUC_Init(RevUpCtrl_Handle_t *pHandle, SpeednTorqCtrl_Handle_t *pSTC, VirtualSpeedSensor_Handle_t *pVSS,
              Bemf_Handle_t *pSNSL, PWMC_Handle_t *pPWM);

/*  Function used to reset internal state of the RevUpCtrl Component to its default state */
void RUC_Clear(RevUpCtrl_Handle_t *pHandle, int16_t hMotorDirection);

/* Main procedure which clock the RevUp controller. */
bool RUC_Exec(RevUpCtrl_Handle_t *pHandle);

/* Return information about current state of programmer RevUp sequence. */
bool RUC_Completed(RevUpCtrl_Handle_t *pHandle);

/* Function allow to stop the programmed RevUp at the current speed. */
void RUC_Stop(RevUpCtrl_Handle_t *pHandle);

/* Check that final acceleration during RevUp phase is reached  */
bool RUC_FirstAccelerationStageReached(RevUpCtrl_Handle_t *pHandle);

/* Function used to set the duration of a specific phase. */
void RUC_SetPhaseDurationms(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, uint16_t hDurationms);

/* Function used to set the targeted motor speed at the end of a specific phase. */
void RUC_SetPhaseFinalMecSpeedUnit(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, int16_t hFinalMecSpeedUnit);

/* Function used to set the final motor torque targeted at the end of a specific phase. */
void RUC_SetPhaseFinalPulse(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase, uint16_t hFinalTorque);

/* Function used to read the duration of a specific RevUp phase */
uint16_t RUC_GetPhaseDurationms(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* Function used to read the targeted mechanical speed of a specific RevUp phase */
int16_t RUC_GetPhaseFinalMecSpeedUnit(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* Function used to read the targeted torque of a specific RevUp phase */
int16_t RUC_GetPhaseFinalPulse(RevUpCtrl_Handle_t *pHandle, uint8_t bPhase);

/* Function used to read the number of phase(s) used by RevUp procedure  */
uint8_t RUC_GetNumberOfPhases(RevUpCtrl_Handle_t *pHandle);

/* It is used to check if this stage is used for align motor. */
uint8_t RUC_IsAlignStageNow(RevUpCtrl_Handle_t * pHandle);

bool RUC_GetPhase(RevUpCtrl_Handle_t *pHandle, uint8_t phaseNumber, RevUpCtrl_PhaseParams_t *phaseData);

bool RUC_SetPhase(RevUpCtrl_Handle_t *pHandle ,uint8_t phaseNumber, RevUpCtrl_PhaseParams_t *phaseData);

/* It is used to check the spinning direction of the motor. */
int16_t RUC_GetDirection(RevUpCtrl_Handle_t * pHandle);

/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif /* __cpluplus */

#endif /* REVUP_CTRL_SIXSTEP_H */

/************************ (C) COPYRIGHT 2022 STMicroelectronics *****END OF FILE****/
