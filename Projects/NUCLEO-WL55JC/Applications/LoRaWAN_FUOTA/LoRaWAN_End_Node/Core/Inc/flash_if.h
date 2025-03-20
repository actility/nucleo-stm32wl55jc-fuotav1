/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    flash_if.h
  * @author  MCD Application Team
  * @brief   This file contains definitions for FLASH Interface functionalities.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2021 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __FLASH_IF_H__
#define __FLASH_IF_H__

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "platform.h"

/* USER CODE BEGIN Includes */
#include "sfu_fwimg_regions.h"
/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/**
  * @brief Flash status
  */
typedef enum
{
  FLASH_IF_PARAM_ERROR  = -6, /*!< Error Flash invalid parameter */
  FLASH_IF_LOCK_ERROR   = -5, /*!< Error Flash not locked */
  FLASH_IF_WRITE_ERROR  = -4, /*!< Error Flash write not possible */
  FLASH_IF_READ_ERROR   = -3, /*!< Error Flash read not possible */
  FLASH_IF_ERASE_ERROR  = -2, /*!< Error Flash erase not possible */
  FLASH_IF_ERROR        = -1, /*!< Error Flash generic */
  FLASH_IF_OK           = 0,  /*!< Flash Success */
  FLASH_IF_BUSY         = 1   /*!< Flash not available */
} FLASH_IF_StatusTypedef;

/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
#define FLASH_IF_BUFFER_SIZE       FLASH_PAGE_SIZE /*!< FLASH Page Size, 2 KBytes */

/* USER CODE BEGIN EC */
#define FLASH_WRITE_ALIGNMENT		8	/*!< FLASH writes should be aligned on 8 bytes */
/* USER CODE END EC */

/* External variables --------------------------------------------------------*/
/* USER CODE BEGIN EV */

/* USER CODE END EV */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */
#define FOTA_SWAP_REGION_START              ((uint32_t)(SlotStartAdd[SLOT_SWAP]))
#define FOTA_SWAP_REGION_SIZE               ((uint32_t)(SlotEndAdd[SLOT_SWAP] - SlotStartAdd[SLOT_SWAP] + 1U))
#define FOTA_SWAP_REGION_END                ((uint32_t)(SlotEndAdd[SLOT_SWAP]))

#define FOTA_DWL_REGION_START               ((uint32_t)(SlotStartAdd[SLOT_DWL_1]))
#define FOTA_DWL_REGION_SIZE                ((uint32_t)(SlotEndAdd[SLOT_DWL_1] - SlotStartAdd[SLOT_DWL_1] + 1U))
#define FOTA_DWL_REGION_END                 ((uint32_t)(SlotEndAdd[SLOT_DWL_1]))

#define FOTA_ACT_REGION_START               ((uint32_t)(SlotStartAdd[SLOT_ACTIVE_1]))
#define FOTA_ACT_REGION_SIZE                ((uint32_t)(SlotEndAdd[SLOT_ACTIVE_1] - SlotStartAdd[SLOT_ACTIVE_1] + 1U))
#define FOTA_ACT_REGION_END                 ((uint32_t)(SlotEndAdd[SLOT_ACTIVE_1])

/* USER CODE END EM */

/* Exported functions prototypes ---------------------------------------------*/
/**
  * @brief This function initializes the internal and external flash interface
  *
  * @param pAllocRamBuffer pointer used to store a FLASH page in RAM when partial replacement is needed
  * @return FLASH_IF_StatusTypedef status
  */
FLASH_IF_StatusTypedef FLASH_IF_Init(void *pAllocRamBuffer);

/**
  * @brief This function de-initializes the internal and external flash interface
  *
  * @return FLASH_IF_StatusTypedef status
  */
FLASH_IF_StatusTypedef FLASH_IF_DeInit(void);

/**
  * @brief This function writes a data buffer in internal or external flash
  *
  * @param pDestination pointer of flash address to write. It has to be 8 bytes aligned.
  * @param pSource pointer on buffer with data to write
  * @param uLength length of data buffer in bytes. It has to be 8 bytes aligned.
  * @return FLASH_IF_StatusTypedef status
  */
FLASH_IF_StatusTypedef FLASH_IF_Write(void *pDestination, const void *pSource, uint32_t uLength);

/**
  * @brief This function reads a amount of data from flash and copy into the output data buffer
  *
  * @param pDestination pointer of target location to copy the flash sector
  * @param pSource pointer of flash address to read
  * @param uLength number of bytes to read
  * @return FLASH_IF_StatusTypedef status
  */
FLASH_IF_StatusTypedef FLASH_IF_Read(void *pDestination, const void *pSource, uint32_t uLength);

/**
  * @brief This function erases a amount of internal or external flash pages depending of the length
  *
  * @param pStart pointer of flash address to erase
  * @param uLength number of bytes to erase
  * @return FLASH_IF_StatusTypedef status
  */
FLASH_IF_StatusTypedef FLASH_IF_Erase(void *pStart, uint32_t uLength);

/* USER CODE BEGIN EFP */

/**
 * @brief This function returns page index in internal or external flash
 *
 * @param uStart flash address to get page index
 * @return Index of page within given address
 */
uint32_t FLASH_IF_PAGE_Index (uint32_t uStart);

/**
 * @brief This function returns address of the page from internal flash
 *
 * @param uIndex flash page index to get address
 * @return Address of the page with given uIndex
 */
uint32_t FLASH_IF_EXT_Page_Address (uint32_t uIndex);

/**
 * @brief Check if address is in external flash
 *
 * @param uStart address in question
 * @return If external flash true
 */
bool FLASH_IF_IsExt (uint32_t uStart);

/**
 * @brief Calculate CRC32 of flash content of internal or external flash
 *
 * @param uStart Flash address to start calculation
 * @param uLength Length of area to calculate CRC32
 * @return CRC32 value
 */
uint32_t FLASH_IF_CRC32 (uint32_t uStart, uint32_t uLength);

/**
 * @brief Get flash page size
 *
 * @param None
 * @return flash page size in bytes
 */
uint32_t FLASH_IF_Page_Size (void);

/**
 * @brief Get alignment of flash write calls
 *
 * @param None
 * @return alignment of flash write calls in bytes
 */
uint32_t FLASH_IF_Alignment_Size (void);

/**
 * @brief Get ACTIVE region start address
 *
 * @param None
 * @return ACTIVE region start address
 */
uint32_t FLASH_IF_Active_Start (void);

/**
 * @brief Get size of ACTIVE region in bytes
 *
 * @param None
 * @return Returns size of ACTIVE region in bytes
 */
uint32_t FLASH_IF_Active_Size (void);

/**
 * @brief Get ACTIVE region start address
 *
 * @param None
 * @return ACTIVE region start address
 */
uint32_t FLASH_IF_Download_Start (void);

/**
 * @brief Get size of DOWNLOAD region in bytes
 *
 * @param None
 * @return Returns size of DOWNLOAD region in bytes
 */
uint32_t FLASH_IF_Download_Size (void);

/**
 * @brief Get ACTIVE region start address
 *
 * @param None
 * @return ACTIVE region start address
 */
uint32_t FLASH_IF_Swap_Start (void);

/**
 * @brief Get size of SWAP region in bytes
 *
 * @param None
 * @return Returns size of SWAP region in bytes
 */
uint32_t FLASH_IF_Swap_Size (void);

/**
 * @brief Get size of file FW vendor header in bytes
 *
 * @param None
 * @return Returns size of header in bytes
 */
uint32_t FLASH_IF_Image_Offset (void);

/* USER CODE END EFP */

#ifdef __cplusplus
}
#endif

#endif /* __FLASH_IF_H__ */
