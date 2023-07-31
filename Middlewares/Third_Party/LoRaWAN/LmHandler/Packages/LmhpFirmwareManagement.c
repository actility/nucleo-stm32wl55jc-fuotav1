/**
  ******************************************************************************
  * @file    LmhpFirmwareManagement.c
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
/* Includes ------------------------------------------------------------------*/
#include "platform.h"
#include "LmHandler.h"
#include "LmhpFirmwareManagement.h"
#include "mw_log_conf.h"  /* needed for MW_LOG */
#if (ACTILITY_SMARTDELTA == 1)
#include "lora_app_version.h"
#include "frag_decoder_if.h"
#include "fw_update_agent.h"
#include "sys_app.h"
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Private typedef -----------------------------------------------------------*/
/*!
 * Package current context
 */
typedef struct LmhpFirmwareManagementState_s
{
  bool Initialized;
  bool IsRunning;
  uint8_t DataBufferMaxSize;
  uint8_t *DataBuffer;
#if (ACTILITY_SMARTDELTA == 1)
  bool DevVersionAnsSentOnBoot;
  bool IsRebootScheduled;
  LmhpFirmwareManagementUpImageStatus_t NewImageStatus;
#endif /* ACTILITY_SMARTDELTA == 1 */
} LmhpFirmwareManagementState_t;

typedef enum LmhpFirmwareManagementMoteCmd_e
{
  FW_MANAGEMENT_PKG_VERSION_ANS              = 0x00,
  FW_MANAGEMENT_DEV_VERSION_ANS              = 0x01,
  FW_MANAGEMENT_DEV_REBOOT_TIME_ANS          = 0x02,
  FW_MANAGEMENT_DEV_REBOOT_COUNTDOWN_ANS     = 0x03,
  FW_MANAGEMENT_DEV_UPGRADE_IMAGE_ANS        = 0x04,
  FW_MANAGEMENT_DEV_DELETE_IMAGE_ANS         = 0x05,
} LmhpFirmwareManagementMoteCmd_t;

typedef enum LmhpFirmwareManagementSrvCmd_e
{
  FW_MANAGEMENT_PKG_VERSION_REQ              = 0x00,
  FW_MANAGEMENT_DEV_VERSION_REQ              = 0x01,
  FW_MANAGEMENT_DEV_REBOOT_TIME_REQ          = 0x02,
  FW_MANAGEMENT_DEV_REBOOT_COUNTDOWN_REQ     = 0x03,
  FW_MANAGEMENT_DEV_UPGRADE_IMAGE_REQ        = 0x04,
  FW_MANAGEMENT_DEV_DELETE_IMAGE_REQ         = 0x05,
} LmhpFirmwareManagementSrvCmd_t;

/* Private define ------------------------------------------------------------*/
/*!
 * LoRaWAN Application Layer Remote multicast setup Specification
 */
#define FW_MANAGEMENT_PORT                          203
#define FW_MANAGEMENT_ID                            4
#define FW_MANAGEMENT_VERSION                       1
#if (ACTILITY_SMARTDELTA == 1)
#define FW_VERSION                                  __APP_VERSION
#else
#define FW_VERSION                                  0x00000000 /* Not yet managed */
#endif /* ACTILITY_SMARTDELTA == 1 */
#define HW_VERSION                                  0x00000000 /* Not yet managed */

#if (ACTILITY_SMARTDELTA == 1)
/*
 * Originally used BlkAckDelay, but we can't transfer it through
 * reboot so define it here and use this parameters
 */
#define DEVVERSIONANSMIN                            16  /* min delay seconds */
#define DEVVERSIONANSMAX                            128 /* max delay seconds */
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Private macro -------------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/*!
 * Initializes the package with provided parameters
 *
 * \param [in] params            Pointer to the package parameters
 * \param [in] dataBuffer        Pointer to main application buffer
 * \param [in] dataBufferMaxSize Main application buffer maximum size
 */
static void LmhpFirmwareManagementInit(void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize);

/*!
 * Returns the current package initialization status.
 *
 * \retval status Package initialization status
 *                [true: Initialized, false: Not initialized]
 */
static bool LmhpFirmwareManagementIsInitialized(void);

/*!
 * Returns the package operation status.
 *
 * \retval status Package operation status
 *                [true: Running, false: Not running]
 */
static bool LmhpFirmwareManagementIsRunning(void);

/*!
 * Processes the internal package events.
 */
static void LmhpFirmwareManagementProcess(void);

/*!
 * Processes the MCPS Indication
 *
 * \param [in] mcpsIndication     MCPS indication primitive data
 */
static void LmhpFirmwareManagementOnMcpsIndication(McpsIndication_t *mcpsIndication);

static void OnRebootTimer(void *context);

#if ( ACTILITY_SMARTDELTA == 1 )
static void OnDevVersionAnsTimerEvent( void *context );
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Private variables ---------------------------------------------------------*/
static LmhpFirmwareManagementState_t LmhpFirmwareManagementState =
{
  .Initialized = false,
  .IsRunning =   false,
#if ( ACTILITY_SMARTDELTA == 1 )
  .DevVersionAnsSentOnBoot = false,
  .IsRebootScheduled = false,
  .NewImageStatus = FW_MANAGEMENT_VALID_IMAGE,
#endif /* ACTILITY_SMARTDELTA == 1 */
};

static LmhPackage_t LmhpFirmwareManagementPackage =
{
  .Port =                       FW_MANAGEMENT_PORT,
  .Init =                       LmhpFirmwareManagementInit,
  .IsInitialized =              LmhpFirmwareManagementIsInitialized,
  .IsRunning =                  LmhpFirmwareManagementIsRunning,
  .Process =                    LmhpFirmwareManagementProcess,
  .OnMcpsConfirmProcess =       NULL,                              /* Not used in this package */
  .OnMcpsIndicationProcess =    LmhpFirmwareManagementOnMcpsIndication,
  .OnMlmeConfirmProcess =       NULL,                              /* Not used in this package */
  .OnJoinRequest =              NULL,                              /* To be initialized by LmHandler */
  .OnSendRequest =              NULL,                              /* To be initialized by LmHandler */
  .OnDeviceTimeRequest =        NULL,                              /* To be initialized by LmHandler */
  .OnPackageProcessEvent =      NULL,                              /* To be initialized by LmHandler */
};

/*!
 * Reboot timer
 */
static TimerEvent_t RebootTimer;

#if ( ACTILITY_SMARTDELTA == 1 )
/*!
 * DevVersionAns at boot timer
 */
static TimerEvent_t DevVersionAnsTimer;
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Exported functions ---------------------------------------------------------*/
LmhPackage_t *LmhpFirmwareManagementPackageFactory(void)
{
  return &LmhpFirmwareManagementPackage;
}

#if (ACTILITY_SMARTDELTA == 1)
bool LmhpFirmwareManagementIsRebootScheduled(void)
{
  return LmhpFirmwareManagementState.IsRebootScheduled;
}

LmhpFirmwareManagementUpImageStatus_t LmhpFirmwareManagementGetImageStatus(void)
{
  return LmhpFirmwareManagementState.NewImageStatus;
}

void LmhpFirmwareManagementSetImageStatus(LmhpFirmwareManagementUpImageStatus_t imagestatus) {
  LmhpFirmwareManagementState.NewImageStatus = imagestatus;
}
#endif /* ACTILITY_SMARTDELTA == 1 */

/* Private  functions ---------------------------------------------------------*/
static void LmhpFirmwareManagementInit(void *params, uint8_t *dataBuffer, uint8_t dataBufferMaxSize)
{
  if (dataBuffer != NULL)
  {
    LmhpFirmwareManagementState.DataBuffer = dataBuffer;
    LmhpFirmwareManagementState.DataBufferMaxSize = dataBufferMaxSize;
    LmhpFirmwareManagementState.Initialized = true;
    LmhpFirmwareManagementState.IsRunning = true;
#if (ACTILITY_SMARTDELTA == 1)
    LmhpFirmwareManagementState.DevVersionAnsSentOnBoot = false;
    LmhpFirmwareManagementState.IsRebootScheduled = false;
    LmhpFirmwareManagementState.NewImageStatus = FW_MANAGEMENT_NO_PRESENT_IMAGE;
#endif /* ACTILITY_SMARTDELTA == 1 */
    TimerInit(&RebootTimer, OnRebootTimer);
  }
  else
  {
    LmhpFirmwareManagementState.IsRunning = false;
    LmhpFirmwareManagementState.Initialized = false;
  }
}

static bool LmhpFirmwareManagementIsInitialized(void)
{
  return LmhpFirmwareManagementState.Initialized;
}

static bool LmhpFirmwareManagementIsRunning(void)
{
  if (LmhpFirmwareManagementState.Initialized == false)
  {
    return false;
  }

  return LmhpFirmwareManagementState.IsRunning;
}

#if (ACTILITY_SMARTDELTA == 1)
static void OnDevVersionAnsTimerEvent( void *context )
{
  uint8_t dataBufferIndex = 0;

  MibRequestConfirm_t mibReq;

  mibReq.Type = MIB_DEVICE_CLASS;
  LoRaMacMibGetRequestConfirm( &mibReq );
  if( mibReq.Param.Class != CLASS_A ) {
       /*
        * Do not interfere with CLASS_C or CLASS_B session,
        * just skip sending DevVersionAns if we are not in CLASS_A
        */
        TimerStop(&DevVersionAnsTimer);
        APP_LOG(TS_OFF, VLEVEL_M, "DevVersionAns canceled\r\n" );
        return;
  }

  if( LmHandlerIsBusy( ) == true )
  {
    /* We will reschedule timer in Process() if stack is busy */
    LmhpFirmwareManagementState.DevVersionAnsSentOnBoot = false;
    return;
  }
  TimerStop(&DevVersionAnsTimer);
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_VERSION_ANS;
  /* FW Version */
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 0) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 8) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 16) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 24) & 0xFF;
  /* HW Version */
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 0) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 8) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 16) & 0xFF;
  LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 24) & 0xFF;
  LmHandlerAppData_t appData = {
    .Buffer = LmhpFirmwareManagementState.DataBuffer,
    .BufferSize = dataBufferIndex,
    .Port = FW_MANAGEMENT_PORT
  };
  LmhpFirmwareManagementPackage.OnSendRequest(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);
  APP_LOG(TS_OFF, VLEVEL_M, "DevVersionAns sent\r\n" );
}
#endif /* ACTILITY_SMARTDELTA == 1 */

static void LmhpFirmwareManagementProcess(void)
{
#if (ACTILITY_SMARTDELTA == 1)
  int32_t delay = randr(DEVVERSIONANSMIN, DEVVERSIONANSMAX) * 1000;

  // We have to send DevVersionAns on boot after random delay just once
  if( LmhpFirmwareManagementState.DevVersionAnsSentOnBoot == false )
  {
    TimerInit( &DevVersionAnsTimer, OnDevVersionAnsTimerEvent );
    TimerSetValue( &DevVersionAnsTimer, delay );
    TimerStart( &DevVersionAnsTimer );
    LmhpFirmwareManagementState.DevVersionAnsSentOnBoot = true;
    APP_LOG(TS_OFF, VLEVEL_M, "DevVersionAns scheduled in %ums\r\n", delay );
  }
#endif /* ACTILITY_SMARTDELTA == 1 */
  /* Not yet implemented */
}

static void LmhpFirmwareManagementOnMcpsIndication(McpsIndication_t *mcpsIndication)
{
  uint8_t cmdIndex = 0;
  uint8_t dataBufferIndex = 0;

  while (cmdIndex < mcpsIndication->BufferSize)
  {
    switch (mcpsIndication->Buffer[cmdIndex++])
    {
      case FW_MANAGEMENT_PKG_VERSION_REQ:
      {
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_PKG_VERSION_ANS;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_ID;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_VERSION;
        break;
      }
      case FW_MANAGEMENT_DEV_VERSION_REQ:
      {
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_VERSION_ANS;
        /* FW Version */
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 0) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 8) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 16) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (FW_VERSION >> 24) & 0xFF;
        /* HW Version */
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 0) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 8) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 16) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (HW_VERSION >> 24) & 0xFF;
        break;
      }
      case FW_MANAGEMENT_DEV_REBOOT_TIME_REQ:
      {
        uint32_t rebootTimeReq = 0;
        uint32_t rebootTimeAns = 0;
        rebootTimeReq  = (mcpsIndication->Buffer[cmdIndex++] << 0) & 0x000000FF;
        rebootTimeReq += (mcpsIndication->Buffer[cmdIndex++] << 8) & 0x0000FF00;
        rebootTimeReq += (mcpsIndication->Buffer[cmdIndex++] << 16) & 0x00FF0000;
        rebootTimeReq += (mcpsIndication->Buffer[cmdIndex++] << 24) & 0xFF000000;

        if (rebootTimeReq == 0)
        {
#if ( ACTILITY_SMARTDELTA == 1 )
          if ( LmhpFirmwareManagementState.NewImageStatus == FW_MANAGEMENT_VALID_IMAGE )
          {
            FwUpdateAgent_Run(false);
          }
#endif /* ACTILITY_SMARTDELTA == 1 */
          NVIC_SystemReset();
        }
        else if (rebootTimeReq == 0xFFFFFFFF)
        {
          rebootTimeAns = rebootTimeReq;
          TimerStop(&RebootTimer);
#if ( ACTILITY_SMARTDELTA == 1 )
          LmhpFirmwareManagementState.IsRebootScheduled = false;
#endif /* ACTILITY_SMARTDELTA == 1 */
        }
        else
        {
          SysTime_t curTime = { .Seconds = 0, .SubSeconds = 0 };
          curTime = SysTimeGet();

          rebootTimeAns = rebootTimeReq - curTime.Seconds;
          if (rebootTimeAns > 0)
          {
            /* Start session start timer */
            TimerSetValue(&RebootTimer, rebootTimeAns * 1000);
            TimerStart(&RebootTimer);
#if ( ACTILITY_SMARTDELTA == 1 )
            LmhpFirmwareManagementState.IsRebootScheduled = true;
#endif /* ACTILITY_SMARTDELTA == 1 */
          }
        }

        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_REBOOT_TIME_ANS;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootTimeAns >> 0) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootTimeAns >> 8) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootTimeAns >> 16) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootTimeAns >> 24) & 0xFF;

        break;
      }
      case FW_MANAGEMENT_DEV_REBOOT_COUNTDOWN_REQ:
      {
        uint32_t rebootCountdown = 0;
        rebootCountdown  = (mcpsIndication->Buffer[cmdIndex++] << 0) & 0x000000FF;
        rebootCountdown += (mcpsIndication->Buffer[cmdIndex++] << 8) & 0x0000FF00;
        rebootCountdown += (mcpsIndication->Buffer[cmdIndex++] << 16) & 0x00FF0000;

        if (rebootCountdown == 0)
        {
#if ( ACTILITY_SMARTDELTA == 1 )
          if ( LmhpFirmwareManagementState.NewImageStatus == FW_MANAGEMENT_VALID_IMAGE )
          {
            FwUpdateAgent_Run(false);
          }
#endif /* ACTILITY_SMARTDELTA == 1 */
          NVIC_SystemReset();
        }
        else if (rebootCountdown == 0xFFFFFF)
        {
          TimerStop(&RebootTimer);
#if ( ACTILITY_SMARTDELTA == 1 )
          LmhpFirmwareManagementState.IsRebootScheduled = false;
#endif /* ACTILITY_SMARTDELTA == 1 */
        }
        else
        {
          if (rebootCountdown > 0)
          {
            /* Start session start timer */
            TimerSetValue(&RebootTimer, rebootCountdown * 1000);
            TimerStart(&RebootTimer);
#if ( ACTILITY_SMARTDELTA == 1 )
            LmhpFirmwareManagementState.IsRebootScheduled = false;
#endif /* ACTILITY_SMARTDELTA == 1 */
          }
        }
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_REBOOT_COUNTDOWN_ANS;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootCountdown >> 0) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootCountdown >> 8) & 0xFF;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (rebootCountdown >> 16) & 0xFF;
        break;
      }
      case FW_MANAGEMENT_DEV_UPGRADE_IMAGE_REQ:
      {
        uint32_t imageVersion = 0;
#if ( ACTILITY_SMARTDELTA == 0 )
        uint8_t imageStatus = FW_MANAGEMENT_NO_PRESENT_IMAGE;
#else /* ACTILITY_SMARTDELTA == 1 */
        uint8_t imageStatus = LmhpFirmwareManagementState.NewImageStatus;
#endif
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_UPGRADE_IMAGE_ANS;
        /* No FW present */
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = imageStatus & 0x03;

        if (imageStatus == FW_MANAGEMENT_VALID_IMAGE)
        {
          /* Next FW version (opt) */
          LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (imageVersion >> 0) & 0xFF;
          LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (imageVersion >> 8) & 0xFF;
          LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (imageVersion >> 16) & 0xFF;
          LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = (imageVersion >> 24) & 0xFF;
        }
        break;
      }
      case FW_MANAGEMENT_DEV_DELETE_IMAGE_REQ:
      {
        uint32_t firmwareVersion = 0;
        firmwareVersion  = (mcpsIndication->Buffer[cmdIndex++] << 0) & 0x000000FF;
        firmwareVersion += (mcpsIndication->Buffer[cmdIndex++] << 8) & 0x0000FF00;
        firmwareVersion += (mcpsIndication->Buffer[cmdIndex++] << 16) & 0x00FF0000;
        firmwareVersion += (mcpsIndication->Buffer[cmdIndex++] << 24) & 0xFF000000;
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FW_MANAGEMENT_DEV_DELETE_IMAGE_ANS;
#if ( ACTILITY_SMARTDELTA == 0 )
        /* No valid image present */
        LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FWM_DEL_ERRORNOVALIDIMAGE;
#else /* ACTILITY_SMARTDELTA == 1 */
        APP_LOG(TS_OFF, VLEVEL_M, "Receive DevDeleteImageReq\r\n");
        if( LmhpFirmwareManagementState.NewImageStatus != FW_MANAGEMENT_VALID_IMAGE )
        {
            LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = FWM_DEL_ERRORNOVALIDIMAGE;
            APP_LOG(TS_OFF, VLEVEL_M, "DevDeleteImageReq: No valid image\r\n" );
        } else {
        /*
         * For now always delete existing image. Until decision
         * is made how to track downloaded image version
         *
         * if( LmhpFWManagementParams->NewImageFWVersion == firmwareVersion )
         * {
         */
            FRAG_DECODER_IF_Erase();
            LmhpFirmwareManagementState.DataBuffer[dataBufferIndex++] = 0;
            LmhpFirmwareManagementState.NewImageStatus = FW_MANAGEMENT_NO_PRESENT_IMAGE;
            APP_LOG(TS_OFF, VLEVEL_M, "DevDeleteImageReq: Image deleted\r\n");
          /*
           * } else {
           *     LmhpFWManagementState.DataBuffer[dataBufferIndex++] = FWM_DEL_ERRORINVALIDVERSION;
           *     APP_LOG(TS_OFF, VLEVEL_M, "DevDeleteImageReq: Invalid image version %x vs %x\r\n", firmwareVersion,
           *                     0 );
           * }
           */
         }
#endif /* ACTILITY_SMARTDELTA == 0 */
         break;
      }
      default:
      {
        break;
      }
    }
  }

  if (dataBufferIndex != 0)
  {
    /* Answer commands */
    LmHandlerAppData_t appData =
    {
      .Buffer = LmhpFirmwareManagementState.DataBuffer,
      .BufferSize = dataBufferIndex,
      .Port = FW_MANAGEMENT_PORT
    };
    bool current_dutycycle;
    LmHandlerGetDutyCycleEnable(&current_dutycycle);

    /* force Duty Cycle OFF to this Send */
    LmHandlerSetDutyCycleEnable(false);
    LmhpFirmwareManagementPackage.OnSendRequest(&appData, LORAMAC_HANDLER_UNCONFIRMED_MSG, NULL, true);

    /* restore initial Duty Cycle */
    LmHandlerSetDutyCycleEnable(current_dutycycle);
  }
}

static void OnRebootTimer(void *context)
{
#if ( ACTILITY_SMARTDELTA == 1 )
  if ( LmhpFirmwareManagementState.NewImageStatus == FW_MANAGEMENT_VALID_IMAGE )
  {
    FwUpdateAgent_Run(false);
  }
#endif /* ACTILITY_SMARTDELTA == 1 */
  NVIC_SystemReset();
}
