/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file    frag_decoder_if.c
  * @author  MCD Application Team
  * @brief   Implements the Data Distribution Agent
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under Ultimate Liberty license
  * SLA0044, the "License"; You may not use this file except in compliance with
  * the License. You may obtain a copy of the License at:
  *                             www.st.com/SLA0044
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "frag_decoder_if.h"
#include "sys_app.h"
#include "utilities.h"
#include "flash_if.h"
#include "fw_update_agent.h"
#include "LmHandler.h"

#if (ACTILITY_SMARTDELTA == 1)
#include "fota_storage.h"
#include "fota_patch.h"
#include "LmhpFirmwareManagement.h"
#include "verify_signature.h"
#endif /* ACTILITY_SMARTDELTA == 1 */


/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* External variables ---------------------------------------------------------*/
/* USER CODE BEGIN EV */

/**
 * RAM buffer for page uncompression. Should match flash page size.
 * Should match flash page size specified in FOTA server device profile.
 */
uint8_t ram_buf[FLASH_PAGE_SIZE];

/* USER CODE END EV */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/*!
 * Defines the maximum size for the buffer receiving the fragmentation result.
 *
 * \remark By default frag_decoder_if.h defines:
 *         \ref FRAG_MAX_NB   313
 *         \ref FRAG_MAX_SIZE 216
 *
 *         In interop test mode will be
 *         \ref FRAG_MAX_NB   20
 *         \ref FRAG_MAX_SIZE 50
 *
 *         FileSize = FRAG_MAX_NB * FRAG_MAX_SIZE
 *
 *         If bigger file size is to be received or is fragmented differently
 *         one must update those parameters.
 *
 * \remark  Memory allocation is done at compile time. Several options have to be foreseen
 *          in order to optimize the memory. Will depend of the Memory management used
 *          Could be Dynamic allocation --> malloc method
 *          Variable Length Array --> VLA method
 *          pseudo dynamic allocation --> memory pool method
 *          Other option :
 *          In place of using the caching memory method we can foreseen to have a direct
 *          flash memory copy. This solution will depend of the SBSFU constraint
 *
 */
#define UNFRAGMENTED_DATA_SIZE                      ( FRAG_MAX_NB * FRAG_MAX_SIZE )

/* USER CODE BEGIN PD */

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
/**
  * Fragmentation package callbacks parameters
  */
const LmhpFragmentationParams_t FRAG_DECODER_IF_FragmentationParams =
{
  .DecoderCallbacks =
  {
    .FragDecoderErase = FRAG_DECODER_IF_Erase,
    .FragDecoderWrite = FRAG_DECODER_IF_Write,
    .FragDecoderRead = FRAG_DECODER_IF_Read,
  },
  .OnProgress = FRAG_DECODER_IF_OnProgress,
  .OnDone = FRAG_DECODER_IF_OnDone
};

#if (INTEROP_TEST_MODE == 1)  /*write fragment in RAM - Caching mode*/
/**
  * Un-fragmented data storage.
  */
static uint8_t UnfragmentedData[UNFRAGMENTED_DATA_SIZE];
#endif /* INTEROP_TEST_MODE */

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
#if (INTEROP_TEST_MODE == 1)
/**
  * \brief Computes a CCITT 32 bits CRC
  *
  * \param [IN] buffer   Data buffer used to compute the CRC
  * \param [IN] length   Data buffer length
  *
  * \retval crc          The computed buffer of length CRC
  */
static uint32_t Crc32(uint8_t *buffer, uint16_t length);
#endif /* INTEROP_TEST_MODE == 0 */

/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Exported functions --------------------------------------------------------*/
int32_t FRAG_DECODER_IF_Erase(void)
{
  int32_t status = FLASH_IF_OK;
  /* USER CODE BEGIN FRAG_DECODER_IF_Erase_1 */

  /* USER CODE END FRAG_DECODER_IF_Erase_1 */
#if (INTEROP_TEST_MODE == 1)
  UTIL_MEM_set_8(UnfragmentedData, 0xFF, UNFRAGMENTED_DATA_SIZE);
#else /* INTEROP_TEST_MODE == 0 */
  status = FLASH_IF_Erase((void *)FRAG_DECODER_DWL_REGION_START, FRAG_DECODER_DWL_REGION_SIZE);

#endif /* INTEROP_TEST_MODE */
  /* USER CODE BEGIN FRAG_DECODER_IF_Erase_2 */

  /* USER CODE END FRAG_DECODER_IF_Erase_2 */
  return status;
}

int32_t FRAG_DECODER_IF_Write(uint32_t addr, uint8_t *data, uint32_t size)
{
  int32_t status = FLASH_IF_OK;
  /* USER CODE BEGIN FRAG_DECODER_IF_Write_1 */

  /* USER CODE END FRAG_DECODER_IF_Write_1 */
#if (INTEROP_TEST_MODE == 1)  /*write fragment in RAM - Caching mode*/
  UTIL_MEM_cpy_8(&UnfragmentedData[addr], data, size);
#else /* INTEROP_TEST_MODE == 0 */

  status = FLASH_IF_Write((void *)(FRAG_DECODER_DWL_REGION_START + addr), (const void *)data, size);

  if (status != FLASH_IF_OK)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n.... !! FLASH_IF_WRITE_ERROR: %d !! ....\r\n", status);
  }
#endif /* INTEROP_TEST_MODE */
  /* USER CODE BEGIN FRAG_DECODER_IF_Write_2 */

  /* USER CODE END FRAG_DECODER_IF_Write_2 */
  return status;
}

int32_t FRAG_DECODER_IF_Read(uint32_t addr, uint8_t *data, uint32_t size)
{
  int32_t status = FLASH_IF_OK;
  /* USER CODE BEGIN FRAG_DECODER_IF_Read_1 */

  /* USER CODE END FRAG_DECODER_IF_Read_1 */
#if (INTEROP_TEST_MODE == 1)   /*Read fragment in RAM - Caching mode*/
  UTIL_MEM_cpy_8(data, &UnfragmentedData[addr], size);
#else /* INTEROP_TEST_MODE == 0 */
  FLASH_IF_Read((void *)data, (const void *)(FRAG_DECODER_DWL_REGION_START + addr), size);
#endif /* INTEROP_TEST_MODE */
  /* USER CODE BEGIN FRAG_DECODER_IF_Read_2 */

  /* USER CODE END FRAG_DECODER_IF_Read_2 */
  return status;
}

void FRAG_DECODER_IF_OnProgress(uint16_t fragCounter, uint16_t fragNb, uint8_t fragSize, uint16_t fragNbLost)
{
  /* USER CODE BEGIN FRAG_DECODER_IF_OnProgress_1 */

  /* USER CODE END FRAG_DECODER_IF_OnProgress_1 */
  APP_LOG(TS_OFF, VLEVEL_M, "\r\n....... FRAG_DECODER in Progress .......\r\n");
  APP_LOG(TS_OFF, VLEVEL_M, "RECEIVED    : %5d / %5d Fragments\r\n", fragCounter, fragNb);
  APP_LOG(TS_OFF, VLEVEL_M, "              %5d / %5d Bytes\r\n", fragCounter * fragSize, fragNb * fragSize);
  APP_LOG(TS_OFF, VLEVEL_M, "LOST        :       %7d Fragments\r\n\r\n", fragNbLost);

#if (INTEROP_TEST_MODE == 1)
  BSP_LED_Toggle(LED_BLUE);
#endif /* INTEROP_TEST_MODE == 1 */
  /* USER CODE BEGIN FRAG_DECODER_IF_OnProgress_2 */

  /* USER CODE END FRAG_DECODER_IF_OnProgress_2 */
}

#if ACTILITY_SMARTDELTA == 1
static void ResetDelayTimer_cb(void *context)
{
  APP_LOG(TS_OFF, VLEVEL_M, "\r\ndelayed fw update run...\r\n");
  FwUpdateAgent_Run(true);
}
#endif

void FRAG_DECODER_IF_OnDone(int32_t status, uint32_t size)
{
  uint32_t FileRxCrc;

  /* USER CODE BEGIN FRAG_DECODER_IF_OnDone_1 */

  /* USER CODE END FRAG_DECODER_IF_OnDone_1 */
  APP_LOG(TS_OFF, VLEVEL_M, "\r\n.... FRAG_DECODER Finished ....\r\n");
  APP_LOG(TS_OFF, VLEVEL_M, "STATUS     : %d\r\n", status);

#if (INTEROP_TEST_MODE == 0)
  /* Do a request to Run the Secure boot - The file is already in flash */
#if (ACTILITY_SMARTDELTA == 0)
  /* Do a request to Run the Secure boot - The file is already in flash */
  FwUpdateAgent_Run();
#else  /* ACTILITY_SMARTDELTA == 1 */

  uint8_t *datafile = (uint8_t *)FOTA_DWL_REGION_START;

  if( *(uint32_t *)datafile != FIRMWARE_MAGIC)
  {
    APP_LOG(TS_OFF, VLEVEL_M, "Binary file received, no firmware magic found\r\n");
    goto cleanup;
  }
  datafile += FLASH_IF_Image_Offset ();
  if( size <= FLASH_IF_Image_Offset () )
  {
    APP_LOG(TS_OFF, VLEVEL_M, "File size: %u less then: %u error\r\n", size, FLASH_IF_Image_Offset ());
    goto cleanup;
  }
  if( fota_patch_verify_header((uint32_t)datafile) == SMARTDELTA_OK )
  {
#ifndef NO_CRYPTO
    if( fota_patch_verify_signature (datafile, size - FLASH_IF_Image_Offset ()) == SMARTDELTA_OK ) {
#endif
      APP_LOG(TS_OFF, VLEVEL_M, "Patch size: %u\r\n", size);
      fota_patch_result_t patch_res = fota_patch(size, ram_buf, FLASH_IF_Page_Size());
      if (patch_res == fotaOk) {
        LmhpFirmwareManagementSetImageStatus(FW_MANAGEMENT_VALID_IMAGE);
        APP_LOG(TS_OFF, VLEVEL_M, "\r\n...... Smart Delta to Flash Succeeded  ......\r\n");
      } else if (patch_res == fotaHeaderSignatureUnsupported) {
        LmhpFirmwareManagementSetImageStatus(FW_MANAGEMENT_INCOMPATIBLE_IMAGE);
        APP_LOG(TS_OFF, VLEVEL_M, "...... Patch unrecognized ......\r\n");
      } else {
        LmhpFirmwareManagementSetImageStatus(FW_MANAGEMENT_CORRUPTED_IMAGE);
        APP_LOG(TS_OFF, VLEVEL_M, "Patch error:%d\r\n", patch_res);
      }
#ifndef NO_CRYPTO
    } else {
      APP_LOG(TS_OFF, VLEVEL_M, "Invalid Smart Delta signature\r\n");
      LmhpFirmwareManagementSetImageStatus(FW_MANAGEMENT_INCOMPATIBLE_IMAGE);
    }
#endif
  } else { /* Process full image upgrade */
    LmhpFirmwareManagementSetImageStatus(FW_MANAGEMENT_VALID_IMAGE);
    APP_LOG(TS_OFF, VLEVEL_M, "\r\n...... Full image received OK ......\r\n");
  }
  /*
   * All fragments received and processed. Switch to Class A
   * and signal multicast session is finished
   *
   * TODO: Need to process delayed reboot
   */
cleanup:
#ifdef  SDTEST
  /*
   * For TESTS ONLY !!!
   */
  if ( LmhpFirmwareManagementGetImageStatus() == FW_MANAGEMENT_VALID_IMAGE) {
     FwUpdateAgent_Run(false);
  }
  /********************/
#endif
  LmHandlerRequestClass( CLASS_A );
  if ( LmhpFirmwareManagementIsRebootScheduled() != true &&
      LmhpFirmwareManagementGetImageStatus() == FW_MANAGEMENT_VALID_IMAGE) {
      uint32_t delay = LmhpFragmentationGetTxDelay();
      APP_LOG(TS_OFF, VLEVEL_M, "\r\nfw update scheduled in %ums\r\n", delay);
      if(delay) {
        static TimerEvent_t ResetDelayTimer;
        TimerInit(&ResetDelayTimer, ResetDelayTimer_cb);
        TimerSetValue(&ResetDelayTimer, delay);
        TimerStart(&ResetDelayTimer);
      } else {
        FwUpdateAgent_Run(true);
      }
  }

#endif /* ACTILITY_SMARTDELTA == 0 */

  FileRxCrc = FLASH_IF_CRC32(FRAG_DECODER_DWL_REGION_START, size);
#else
  FileRxCrc = Crc32(UnfragmentedData, size);
#endif /* INTEROP_TEST_MODE == 1 */
  APP_LOG(TS_OFF, VLEVEL_M, "Size       : %05d Bytes\r\n", size);
  APP_LOG(TS_OFF, VLEVEL_M, "CRC        : %08X\r\n\r\n", FileRxCrc);
  /* USER CODE BEGIN FRAG_DECODER_IF_OnDone_2 */

  /* USER CODE END FRAG_DECODER_IF_OnDone_2 */
}

/* USER CODE BEGIN EF */

/* USER CODE END EF */

/* Private Functions Definition -----------------------------------------------*/
#if (INTEROP_TEST_MODE == 1)
static uint32_t Crc32(uint8_t *buffer, uint16_t length)
{
  /* USER CODE BEGIN Crc32_1 */

  /* USER CODE END Crc32_1 */
  const uint32_t reversedPolynom = 0xEDB88320;
  uint32_t crc = 0xFFFFFFFF;
  if (buffer == NULL)
  {
    return 0;
  }
  for (uint16_t i = 0; i < length; ++i)
  {
    crc ^= (uint32_t)buffer[i];
    for (uint16_t i = 0; i < 8; i++)
    {
      crc = (crc >> 1) ^ (reversedPolynom & ~((crc & 0x01) - 1));
    }
  }
  return ~crc;
  /* USER CODE BEGIN Crc32_2 */

  /* USER CODE END Crc32_2 */
}
#endif /* INTEROP_TEST_MODE */

/* USER CODE BEGIN PrFD */

/* USER CODE END PrFD */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
