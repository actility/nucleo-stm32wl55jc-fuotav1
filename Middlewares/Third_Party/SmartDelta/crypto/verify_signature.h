/*  _        _   _ _ _ _
   / \   ___| |_(_) (_) |_ _   _
  / _ \ / __| __| | | | __| | | |
 / ___ \ (__| |_| | | | |_| |_| |
/_/   \_\___|\__|_|_|_|\__|\__, |
                           |___/
    (C)2017 Actility
License: see LICENCE_SLA0ACT.TXT file include in the project
Description: Smart Delta RMC server ECDSA signature verification
*/

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef VERIFY_SIGNATURE_H
#define VERIFY_SIGNATURE_H

#include "fota_storage.h"

#define SMARTDELTA_ASYM_PUBKEY_LEN      ((int32_t) 64)  /*!< Smart Delta Asymmetric Public Key length (bytes)*/

#define SMARTDELTA_ERROR				-1
#define SMARTDELTA_OK					0

#define NO_CRYPTO       /* Do not use cryptographic verification of signature to reduce images size. WL55 only */

/**
  * @brief Smart Delta patch verify signature function.
  *        Authenticates the Smart Delta patch with header.
  * @param patch : Patch body with header plus signature.
  * @param patchSize : Size of Patch
  * @retval SMARTDELTA_OK if successful, SMARTDELTA_ERROR otherwise.
  */
int32_t fota_patch_verify_signature (uint8_t *patch, size_t patchSize);

#endif /* VERIFY_SIGNATURE_H */


