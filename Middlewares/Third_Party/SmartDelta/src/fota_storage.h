/*  _        _   _ _ _ _
   / \   ___| |_(_) (_) |_ _   _
  / _ \ / __| __| | | | __| | | |
 / ___ \ (__| |_| | | | |_| |_| |
/_/   \_\___|\__|_|_|_|\__|\__, |
                           |___/
    (C)2022 Actility
License: see LICENCE_SLA0ACT.TXT file include in the project
Description: FOTA storage handling
*/
#ifndef FOTA_STORAGE_H
#define FOTA_STORAGE_H

/* Includes ------------------------------------------------------------------*/
#include "platform.h"

#include "sfu_fwimg_regions.h"

#include "flash_if.h"
#include "se_def_metadata.h"

/* Exported constants --------------------------------------------------------*/

/*
 * Size of buffer storage used in patch as uncompress buffer
 * and temporary storage to keep contents of rewritten
 * flash page. Should no less then MCU flash page size
 * and match compression ratio in patch processing
 */
#define RAM_STORAGE_SZ   2048

#define FLASH_ALIGN             8       /* Alignment of flash write operations */
#define NEWIMAGE_BUF_SIZE       8       /* Size of temporary NEWIMAGE write buffer to provide 64 bit alignment, multiple of FLSH_ALIGN ! */
#define SMARTDELTA_MAC_LEN      72      /* Size of Smart Delta crypto signature (including padding) */
#define FIRMWARE_MAGIC          0x31554653 /* Firmware file magic to distinguish from binary - "SFU1" */

#define FOTA_SWAP_REGION_START              ((uint32_t)(SlotStartAdd[SLOT_SWAP]))
#define FOTA_SWAP_REGION_SIZE               ((uint32_t)(SlotEndAdd[SLOT_SWAP] - SlotStartAdd[SLOT_SWAP] + 1U))
#define FOTA_SWAP_REGION_END                ((uint32_t)(SlotEndAdd[SLOT_SWAP]))

#define FOTA_DWL_REGION_START               ((uint32_t)(SlotStartAdd[SLOT_DWL_1]))
#define FOTA_DWL_REGION_SIZE                ((uint32_t)(SlotEndAdd[SLOT_DWL_1] - SlotStartAdd[SLOT_DWL_1] + 1U))
#define FOTA_DWL_REGION_END                 ((uint32_t)(SlotEndAdd[SLOT_DWL_1]))

#define FOTA_ACT_REGION_START               ((uint32_t)(SlotStartAdd[SLOT_ACTIVE_1]))
#define FOTA_ACT_REGION_SIZE                ((uint32_t)(SlotEndAdd[SLOT_ACTIVE_1] - SlotStartAdd[SLOT_ACTIVE_1] + 1U))
#define FOTA_ACT_REGION_END                 ((uint32_t)(SlotEndAdd[SLOT_ACTIVE_1])

typedef enum storage_status_s {
    STR_OK = 0,
    STR_BAD_LEN,
    STR_BAD_OFFSET,
    STR_HAL_ERR,
    STR_INCONSISTENCY,
    STR_BADALIGN,
    STR_VER_ERR,
    STR_BUSY,
    STR_NOMEM,
} storage_status_t;

/* Exported functions ------------------------------------------------------- */

/**
  * Writes `data` buffer of `size` to the current position in NEWIMAGE region
  * NEWIMAGE region is virtual region matching start and end of DOWNLOAD region
  * and dedicated to the unpacked image. When Smart Delta is used patch is moved
  * to the end of DOWNLOAD region and unpacked image is stored at the virtual
  * region called NEWIMAGE located from the start of physical DOWNLOAD region.
  * Start location of the moved patch should be aligned on the page boundary
  * to avoid complications when DOWNLOAD region will be erased in preparation
  * of NEWIMAGE unpack.
  * When current pointer in virtual NEWIMAGE region is going to overlap with the
  * start of the page boundary on which current read position in DOWNLOAD area
  * is located special action is performed. Patch chunk starting from this page
  * boundary in DOWNLOAD region and up to the end of the DOWNLOAD region or
  * up to the size of the SWAP region whatever is smaller will be copied to
  * SWAP region and current DOWNLOAD region pointer will be moved to the beginning
  * of SWAP region location.
  *
  * \param [IN] data Data buffer to be written at current NEWIMAGE region pointer.
  * \param [IN] size Size of data buffer to be written.
  * \param [OUT] error Pointer to the errorcode occured if any
  *
  * \retval status Write operation status [-1 Fail (error contains code), >=0 Bytes wrote]
  */
int32_t fota_storage_write(uint8_t *data, uint32_t size, storage_status_t *error);

/**
  * Reads `data` buffer of `size` from the current location of patch source pointer.
  * Patch source could be located either at the end of DOWNLOAD region or in the
  * SWAP region. Latter is required when unpacked image is going to overlap
  * with patch source, in other words when current write pointer in virtual NEWIMAGE
  * area is going to overlap with the current read pointer in DOWNLOAD area.
  * When this situation is detected patch slice starting from current
  * DOWNLOAD read pointer is copied to the SWAP area. See more details in
  * fota_storage_write() description.
  *
  * \param [IN] data Data buffer to be read from current DOWNLOAD region pointer.
  * \param [IN] size Size of data buffer to be read.
  * \param [OUT] error Pointer to the errorcode occured if any
  *
  * \retval status Read operation status [-1 Fail (error contains code), >=0 Bytes read]
  */
int32_t fota_storage_read(uint8_t *data, uint32_t size, storage_status_t *error);

/**
  * Initializes internal data structures and pointers prior to start of
  * Smart Delta patch processing. Move patch from start of DOWNLOAD region
  * to the beginning of highest possible page in DOWNLOAD region.
  *
  * \param [IN]  size Size of the Smart Delta patch in DOWNLOAD region
  * \param [OUT] error Error contains pointer to the code of the error occurred
  *
  * \retval status Read operation status [-1 Fail (error contains code), 0 Success]
  */
int32_t fota_storage_init(uint32_t size, storage_status_t *error);

/**
  * Allocate RAM buffer for use during patch processing
  *
  * \param [OUT] ram_buf pointer where to write ram buffer start pointer
  *
  * \retval size Size of RAM buffer returned or 0 on failure
  */
uint32_t fota_storage_get_rambuf(uint8_t **ram_buf);

/**
  * Virtually write one byte to the flash. As all writes should be aligned
  * on the 64bit boundary just store current byte in RAM until all 64bits
  * are collected and write 64bits into the flash when they are collected.
  *
  * \param [IN] b Byte to be written
  * \param [OUT] error Pointer to the error code occurred if any
  *
  * \retval status Write operation status [-1 Fail (error contains code), 0 Success]
  */
int32_t fota_storage_write_byte (uint8_t b, storage_status_t *error);

/**
  *  Write current temporary buffer of fota_storage_write_byte() function to the flash
  *  when no more bytes are planned to be written. Erase remaining unused portion
  *  of NEWIMAGE/DOWNLOAD region which was made dirty by patch copied to the end of
  *  DOWNLOAD region. Current page of NEWIMAGE (were "newimage_ptr" is located) is
  *  always erased up to the end.
  *
  *  \param [OUT] error Pointer to the error code occurred if any
  *
  *  \retval status Flush operation status [-1 Fail  (error contains code), >=0 NUmber of bytes flushed]
  *
  */
int32_t fota_storage_flush( storage_status_t *error );

/**
  * Get address of the ACTIVE region start
  *
  * \retval address Start of ACTIVE region address
  */
uint32_t fota_storage_get_active_start(void);

/**
  * Get length of the ACTIVE region in bytes
  *
  * \retval length Length of the ACTIVE region in bytes
  */
uint32_t fota_storage_get_active_len(void);

/**
 * Write chunk of arbitrary size data into flash
 * We need it to workaround ST's FLASH_IF_Write() bug which
 * do not properly support multipage writes
 * with page aligned start and page unaligned end.
 *
 * \param [IN] addr Destination address in the flash
 * \param [IN] data Source address
 * \param [IN] size Write size in bytes
 *
 * \retval status of the operation [FLASH_OK or fail code otherwise]
 *
 */
int32_t fota_flash_write(uint32_t addr, uint32_t data, uint32_t size);

#endif /* FOTA_STORAGE_H */
