/**
  ******************************************************************************
  * @file    speed_pos_fdbk.c
  * @author  Motor Control SDK Team, ST Microelectronics
  * @brief   This file provides firmware functions that implement the  features
  *          of the Speed & Position Feedback component of the Motor Control SDK.
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
  */

/* Includes ------------------------------------------------------------------*/
#include "speed_pos_fdbk.h"

/** @addtogroup MCSDK
  * @{
  */

/** @defgroup SpeednPosFdbk Speed & Position Feedback
  *
  * @brief Speed & Position Feedback components of the Motor Control SDK
  *
  * These components provide the speed and the angular position of the rotor of a motor (both
  * electrical and mechanical).
  *
  * Several implementations of the Speed and Position Feedback feature are provided by the Motor
  * to account for the specificities of the motor used on the application:
  *
  * - @ref hall_speed_pos_fdbk "Hall Speed & Position Feedback" for motors with Hall effect sensors
  * - @ref Encoder  "Encoder Speed & Position Feedback" for motors with a quadrature encoder
  * - two general purpose sensorless implementations are provided:
  *   @ref SpeednPosFdbk_STO "State Observer with PLL" and
  *   @ref STO_CORDIC_SpeednPosFdbk "State Observer with CORDIC"
  * - "High Frequency Injection" for anisotropic I-PMSM motors (Not included in this release).
  *
  * @{
  */

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section(".ccmram")))
#endif
#endif
/**
  * @brief  It returns the last computed rotor electrical angle, expressed in
  *         s16degrees. 1 s16degree = 360ï¿½/65536
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval int16_t rotor electrical angle (s16degrees)
  */
__weak int16_t SPD_GetElAngle(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0 : pHandle->hElAngle);
#else
  return (pHandle->hElAngle);
#endif
}

/**
  * @brief  It returns the last computed rotor mechanical angle, expressed in
  *         s16degrees. Mechanical angle frame is based on parameter bElToMecRatio
  *         and, if occasionally provided - through function SPD_SetMecAngle -
  *         of a measured mechanical angle, on information computed thereof.
  * @note   both Hall sensor and Sensor-less do not implement either
  *         mechanical angle computation or acceleration computation.
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval int16_t rotor mechanical angle (s16degrees)
  */
__weak int32_t SPD_GetMecAngle(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0 : pHandle->wMecAngle);
#else
  return (pHandle->wMecAngle);
#endif
}

/**
  * @brief  Returns the last computed average mechanical speed, expressed in
  *         the unit defined by #SPEED_UNIT.
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  */
__weak int16_t SPD_GetAvrgMecSpeedUnit(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0 : pHandle->hAvrMecSpeedUnit);
#else
  return (pHandle->hAvrMecSpeedUnit);
#endif
}

/**
  * @brief  It returns the last computed electrical speed, expressed in Dpp.
  *         1 Dpp = 1 s16Degree/control Period. The control period is the period
  *         on which the rotor electrical angle is computed (through function
  *         SPD_CalcElectricalAngle).
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval int16_t rotor electrical speed (Dpp)
  */
__weak int16_t SPD_GetElSpeedDpp(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0 : pHandle->hElSpeedDpp);
#else
  return (pHandle->hElSpeedDpp);
#endif
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section(".ccmram")))
#endif
#endif
/**
  * @brief  It returns the last instantaneous computed electrical speed, expressed in Dpp.
  *         1 Dpp = 1 s16Degree/control Period. The control period is the period
  *         on which the rotor electrical angle is computed (through function
  *         SPD_CalcElectricalAngle).
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval int16_t rotor instantaneous electrical speed (Dpp)
  */
__weak int16_t SPD_GetInstElSpeedDpp(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0 : pHandle->InstantaneousElSpeedDpp);
#else
  return (pHandle->InstantaneousElSpeedDpp);
#endif
}

/**
  * @brief  It returns the result of the last reliability check performed.
  *         Reliability is measured with reference to parameters
  *         hMaxReliableElSpeedUnit, hMinReliableElSpeedUnit,
  *         bMaximumSpeedErrorsNumber and/or specific parameters of the derived
  *         true = sensor information is reliable
  *         false = sensor information is not reliable
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval bool sensor reliability state
  */
__weak bool SPD_Check(const SpeednPosFdbk_Handle_t *pHandle)
{
  bool SpeedSensorReliability = true;
#ifdef NULL_PTR_SPD_POS_FBK
  if ((MC_NULL == pHandle) || (pHandle->bSpeedErrorNumber == pHandle->bMaximumSpeedErrorsNumber))
#else
  if (pHandle->bSpeedErrorNumber == pHandle->bMaximumSpeedErrorsNumber)
#endif
  {
    SpeedSensorReliability = false;
  }
  return (SpeedSensorReliability);
}

#if defined (CCMRAM)
#if defined (__ICCARM__)
#pragma location = ".ccmram"
#elif defined (__CC_ARM) || defined(__GNUC__)
__attribute__((section(".ccmram")))
#endif
#endif

/**
  * @brief  This method must be called - at least - with the same periodicity
  *         on which speed control is executed. It computes and returns - through
  *         parameter pMecSpeedUnit - the rotor average mechanical speed,
  *         expressed in the unit defined by #SPEED_UNIT. It computes and returns
  *         the reliability state of the sensor; reliability is measured with
  *         reference to parameters hMaxReliableElSpeedUnit, hMinReliableElSpeedUnit,
  *         bMaximumSpeedErrorsNumber and/or specific parameters of the derived
  *         true = sensor information is reliable
  *         false = sensor information is not reliable
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @param  pMecSpeedUnit pointer to int16_t, used to return the rotor average
  *         mechanical speed (expressed in the unit defined by #SPEED_UNIT)
  * @retval none
  */
__weak bool SPD_IsMecSpeedReliable(SpeednPosFdbk_Handle_t *pHandle, const int16_t *pMecSpeedUnit)
{
  bool SpeedSensorReliability = true;
#ifdef NULL_PTR_SPD_POS_FBK
  if ((MC_NULL == pHandle) || (MC_NULL == pMecSpeedUnit))
  {
    SpeedSensorReliability = false;
  }
  else
  {
#endif
    uint16_t hAbsMecSpeedUnit;
    uint16_t hAbsMecAccelUnitP;
    int16_t hAux;
    uint8_t bSpeedErrorNumber;
    uint8_t bMaximumSpeedErrorsNumber = pHandle->bMaximumSpeedErrorsNumber;
    bool SpeedError = false;

    bSpeedErrorNumber = pHandle->bSpeedErrorNumber;

    /* Compute absoulte value of mechanical speed */
    if (*pMecSpeedUnit < 0)
    {
      hAux = -(*pMecSpeedUnit);
      hAbsMecSpeedUnit = (uint16_t)hAux;
    }
    else
    {
      hAbsMecSpeedUnit = (uint16_t)(*pMecSpeedUnit);
    }

    if (hAbsMecSpeedUnit > pHandle->hMaxReliableMecSpeedUnit)
    {
      SpeedError = true;
    }

    if (hAbsMecSpeedUnit < pHandle->hMinReliableMecSpeedUnit)
    {
      SpeedError = true;
    }

    /* Compute absoulte value of mechanical acceleration */
    if (pHandle->hMecAccelUnitP < 0)
    {
      hAux = -(pHandle->hMecAccelUnitP);
      hAbsMecAccelUnitP = (uint16_t)hAux;
    }
    else
    {
      hAbsMecAccelUnitP = (uint16_t)pHandle->hMecAccelUnitP;
    }

    if (hAbsMecAccelUnitP > pHandle->hMaxReliableMecAccelUnitP)
    {
      SpeedError = true;
    }

    if (true == SpeedError)
    {
      if (bSpeedErrorNumber < bMaximumSpeedErrorsNumber)
      {
        bSpeedErrorNumber++;
      }
    }
    else
    {
      if (bSpeedErrorNumber < bMaximumSpeedErrorsNumber)
      {
        bSpeedErrorNumber = 0u;
      }
    }

    if (bSpeedErrorNumber == bMaximumSpeedErrorsNumber)
    {
      SpeedSensorReliability = false;
    }

    pHandle->bSpeedErrorNumber = bSpeedErrorNumber;
#ifdef NULL_PTR_SPD_POS_FBK
  }
#endif
  return (SpeedSensorReliability);
}

/**
  * @brief  This method returns the average mechanical rotor speed expressed in
  *         "S16Speed". It means that:\n
  *         - it is zero for zero speed,\n
  *         - it become INT16_MAX when the average mechanical speed is equal to
  *           hMaxReliableMecSpeedUnit,\n
  *         - it becomes -INT16_MAX when the average mechanical speed is equal to
  *         -hMaxReliableMecSpeedUnit.
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval int16_t The average mechanical rotor speed expressed in "S16Speed".
  */
__weak int16_t SPD_GetS16Speed(const SpeednPosFdbk_Handle_t *pHandle)
{
  int16_t tempValue;
#ifdef NULL_PTR_SPD_POS_FBK
  if (MC_NULL == pHandle)
  {
    tempValue = 0;
  }
  else
  {
#endif
    int32_t wAux = (int32_t)pHandle->hAvrMecSpeedUnit;
    wAux *= INT16_MAX;
    wAux /= (int16_t)pHandle->hMaxReliableMecSpeedUnit;
    tempValue = (int16_t)wAux;
#ifdef NULL_PTR_SPD_POS_FBK
  }
#endif
  return (tempValue);
}

/**
  * @brief  This method returns the coefficient used to transform electrical to
  *         mechanical quantities and viceversa. It usually coincides with motor
  *         pole pairs number.
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @retval uint8_t The motor pole pairs number.
  */
__weak uint8_t SPD_GetElToMecRatio(const SpeednPosFdbk_Handle_t *pHandle)
{
#ifdef NULL_PTR_SPD_POS_FBK
  return ((MC_NULL == pHandle) ? 0U : pHandle->bElToMecRatio);
#else
  return (pHandle->bElToMecRatio);
#endif
}

/**
  * @brief  This method sets the coefficient used to transform electrical to
  *         mechanical quantities and viceversa. It usually coincides with motor
  *         pole pairs number.
  * @param  pHandle: handler of the current instance of the SpeednPosFdbk component
  * @param  bPP The motor pole pairs number to be set.
  */
__weak void SPD_SetElToMecRatio(SpeednPosFdbk_Handle_t *pHandle, uint8_t bPP)
{
#ifdef NULL_PTR_SPD_POS_FBK
  if (MC_NULL == pHandle)
  {
    /* Nothing to do */
  }
  else
  {
#endif
    pHandle->bElToMecRatio = bPP;
#ifdef NULL_PTR_SPD_POS_FBK
  }
#endif
}


/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT 2022 STMicroelectronics *****END OF FILE****/
