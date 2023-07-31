/*  _        _   _ _ _ _
   / \   ___| |_(_) (_) |_ _   _
  / _ \ / __| __| | | | __| | | |
 / ___ \ (__| |_| | | | |_| |_| |
/_/   \_\___|\__|_|_|_|\__|\__, |
                           |___/
    (C)2017 Actility
License: see LICENCE_SLA0ACT.TXT file include in the project
Description: FOTA Image Patching Definitions
*/

#ifndef FOTA_PATCH_DEFS_H
#define FOTA_PATCH_DEFS_H
//----------------------------------------------------------------------------
#include <stddef.h>
#include <stdint.h>
//#include <stdbool.h>
//----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C" {
#endif
//----------------------------------------------------------------------------
// PATCH HEADERS
#define FOTA_SIGNATURE_SIZE 8

typedef struct __attribute__((packed)) {
    char Signature[FOTA_SIGNATURE_SIZE];
#define FOTA_SIGNATURE_IMG0 "FOTAI0\0\0" // raw image
    //TBD
}  fota_header_fotai0_t;

#define PACK_WINDOW_ORDER_MAX 31 // 5 bits

typedef struct __attribute__((packed)) {
    char Signature[FOTA_SIGNATURE_SIZE];
#define FOTA_SIGNATURE_IMG1 "FOTAI1\0\0" // LZG packed image
    uint8_t PackWindowOrderI1:5;
    uint8_t flags:3;
}  fota_header_fotai1_t;

typedef struct __attribute__((packed)) /*__attribute__((gcc_struct)) or -mno-ms-bitfields*/ {
    char Signature[FOTA_SIGNATURE_SIZE];
#define FOTA_SIGNATURE_PATCH1 "FOTAP1\0\0" // LZG packed 0-squeezed bsdiff32 patch
    uint8_t PackWindowOrderP1:5;
    int     ZeroNotSqueezed:1;
    uint8_t flags2:2;
    uint32_t OriginalLength;
    uint32_t OriginalHash;
} fota_header_fotap1_t;
//----------------------------------------------------------------------------
typedef enum {
    fotaOk = 0,
    fotaHeaderSignatureSmall    =   1, // input data format errors
    fotaHeaderSmall,
    fotaHeaderSignatureUnsupported,
    fotaHeaderParsBad,
    fotaPatchOrigHash          =  10, // prerequisites errors
    fotaPatchOrigSmall,
    fotaPackLevelBig,
    fotaLzgMiss                 =  20, // algorithm errors
    fotaBspatchOrigMiss,
    fotaTargetOverflow          =  30, // output errors
    fotaTargetWriteFailed,
    fotaError                   = 100, // general error
} fota_patch_result_t;
//----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//----------------------------------------------------------------------------
#endif // FOTA_PATCH_DEFS_H
