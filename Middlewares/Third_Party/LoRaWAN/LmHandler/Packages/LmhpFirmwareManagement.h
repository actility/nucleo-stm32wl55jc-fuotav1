/**
  ******************************************************************************
  * @file    LmhpFirmwareManagement.h
  * @author  MCD Application Team
  * @brief   Implements the LoRa-Alliance Firmware Management package
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
#ifndef __LMHP_FIRMWARE_MANAGEMENT_H__
#define __LMHP_FIRMWARE_MANAGEMENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "LmhPackage.h"

/* Exported defines ----------------------------------------------------------*/
/*!
 * Firmware Management package identifier.
 *
 * \remark This value must be unique amongst the packages
 */
#define PACKAGE_ID_FIRMWARE_MANAGEMENT               4

/* Exported constants --------------------------------------------------------*/
#if ( ACTILITY_SMARTDELTA == 1 )
#define FWM_DEL_ERRORINVALIDVERSION             0x0002
#define FWM_DEL_ERRORNOVALIDIMAGE               0x0001
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Exported types ------------------------------------------------------------*/
#if ( ACTILITY_SMARTDELTA == 1 )
typedef enum LmhpFirmwareManagementUpImageStatus_e
{
  FW_MANAGEMENT_NO_PRESENT_IMAGE             = 0x00,
  FW_MANAGEMENT_CORRUPTED_IMAGE              = 0x01,
  FW_MANAGEMENT_INCOMPATIBLE_IMAGE           = 0x02,
  FW_MANAGEMENT_VALID_IMAGE                  = 0x03,
} LmhpFirmwareManagementUpImageStatus_t;
#endif /* ACTILITY_SMARTDELTA == 1 */

/* External variables --------------------------------------------------------*/
/* Exported macros -----------------------------------------------------------*/
/* Exported functions ------------------------------------------------------- */
LmhPackage_t *LmhpFirmwareManagementPackageFactory(void);

#if (ACTILITY_SMARTDELTA == 1)
/**
  * Get information weither reboot scheduled or not
  *
  * \retval scheduled true - reboot is scheduled, false - reboot is not scheduled
  */
bool LmhpFirmwareManagementIsRebootScheduled(void);

/**
  * Get information about new firmware image loaded in the flash
  *
  * \retval imagestatus
  */
LmhpFirmwareManagementUpImageStatus_t LmhpFirmwareManagementGetImageStatus(void);

/**
  * Set information about status of the firmware image loaded in the flash
  */
void LmhpFirmwareManagementSetImageStatus(LmhpFirmwareManagementUpImageStatus_t imagestatus);
#endif /* ACTILITY_SMARTDELTA == 1 */

#ifdef __cplusplus
}
#endif

#endif /* __LMHP_FIRMWARE_MANAGEMENT_H__ */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
