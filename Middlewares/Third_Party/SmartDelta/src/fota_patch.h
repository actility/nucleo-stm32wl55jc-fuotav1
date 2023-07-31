/*  _        _   _ _ _ _
   / \   ___| |_(_) (_) |_ _   _
  / _ \ / __| __| | | | __| | | |
 / ___ \ (__| |_| | | | |_| |_| |
/_/   \_\___|\__|_|_|_|\__|\__, |
                           |___/
    (C)2017 Actility
License: see LICENCE_SLA0ACT.TXT file include in the project
Description: FOTA Firmware Patching Support
*/

#ifndef __PATCH_H__
#define __PATCH_H__
//----------------------------------------------------------------------------
#include <stdint.h>
#include "fota_patch_defs.h"
//----------------------------------------------------------------------------
/**
 * \brief Get patch located at the beginning of DOWALOAD region and unpack
 * it into NEWIMAGE region which is physically the same as DOWNLOAD region.
 * To avoid overlapping patch prior to unpacking is moved to the very top
 * of DOWNLOAD region and virtual NEWIMAGE region is located from the start
 * of DOWNLOAD region. if NEWIMAGE region during unpacking reaches DOWNLOAD region
 * patch chunk is moved to SWAP region to avoid overlapping.
 *
 * \param len Length of total patch binary received (including header and signature trailer)
 * \retval Result of patch unpacking. See fota_patch_result_t type definition in fota_patch_defs.h
 */
fota_patch_result_t fota_patch(uint32_t len);

/**
 * \brief Verify patch magic to decide if binary just received could
 * be smart delta patch
 *
 * \param fwimagebody Pointer to the area with possible smart delta patch
 * \retval SMARTDELTA_OK if successful, SMARTDELTA_ERROR otherwise.
 */
int32_t fota_patch_verify_header (const uint8_t * fwimagebody);

/**
  * \brief Process smart delta patch after all fota fragments are received
  *
  * \param status Status of fragments transfer
  * \param size Length of file received
  */
void fota_frag_decoder_if_ondone(int32_t status, uint32_t size);
//----------------------------------------------------------------------------
#endif /* __PATCH_H__ */

