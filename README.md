# Introduction

This git repository contains embedded firmware project which provides LoRaWAN
FUOTA (Firmware Update Over The Air) functionality using Actility FUOTA service.  
Device code supports full firmware update and Smart Delta incremental update.  
Device code currently supports only NUCLEO-WL55JC1 & NUCLEO-WL55JC2 hardware platforms but could be
ported to other compatible platforms. Only EU868, US916 and AS923 ISM bands are
supported. This is limitation of the FUOTA service. Support of the other bands
will be introduced later.  
This device code is based on the ST's “STM32Cube MCU Package for STM32WL series”
v1.3.0, available
[here](https://github.com/STMicroelectronics/STM32CubeWL).

# New features, improvements and limitations 

This device code contains following new features, improvements and limitations,
comparing to ST’s “STM32Cube MCU Package for STM32WL series” v1.3.0:

*New features*

-   **Smart Delta incremental firmware update.** Smart Delta is an intelligent
    type of firmware upgrade compression which allows to transfer during the
    upgrade session only changed pieces of device code. It does not require any
    special preparation and planning of firmware image comparing for example
    with partial update functionality when only contiguous specially crafted
    regions of device firmware could be upgraded.
	Improved flash storage management which is part of the Smart Delta functionality 
	allows unpack of the Smrt Delta patch into the same flash area where it was 
	received minimizing flash amount requirements for Smart Delta firmware upgrade
	campaigns.

-   Updated **LoRaWAN Firmware Management Protocol support** 
    to be compatible with Actility FUOTA service.


*Limitations*

-   **STM original default project configuration not supported**. Not tested.

-   **Only STM32CubeIDE project is currently supported**. 

# Hardware and software requirements

*Board requirements*

Currently device code only supports STM32WL55JC MCU. List of supported platforms will be extended in the future.

In order to run device code, get following part:

-   **[ST MCU development board NUCLEO-WL55JC1](https://www.st.com/en/evaluation-tools/nucleo-wl55jc.html).**
    
*MCU flash requirements*

Device code implementation currently requires 3 equally sized flash regions to
support firmware update. These regions are:

-   **SLOT0** – active firmware image slot

-   **SLOT1** – new firmware image slot. SBSFU bootloader will check presence of
    new firmware in this slot and if it is valid will install it in SLOT0. 
	Smart Delta patch is received into this slot. Upon reception it is moved to 
	the top of the slot area and new firmware image is unpacked starting from 
	the beginning of the slot. If unpacked image and patch are overlapped at some 
	moment remaining part of the Smart Delta patched is moved to the SWAP slot.

-   **SWAP** – area which is used for swapping SLOT1 and SLOT0 contents during image
    installation and rollback (if installation fails). It also accomodates part of
	Smart Delta patch if it is overlapped with new image body during unpacking process.
	Its size should be set to accomodate possible overlap portion of Smart Delta, 
	typically to 4K.

Default configuration of device code sets size of SLOT0 and SLOT1 to 91K
and SWAP to 4K. For more details about memory mapping planning 
see “Appendix B. Porting”.

*Fragmentation algorithm RAM overhead calculation*

Memory overhead of the fragmentation algorithm support and Smart Delta library
support could be calculated as following.

The amount of memory required for the algorithm depends on:

-   Number of fragments required for a full file (without the redundancy
    packets) (nbFrag). Default configuration nbFrag = 1000.

-   The size of a data fragment (fragSize). Default configuration fragSize =
    230.

-   The maximum number of redundancy frames that are expected (nbRedundancy).
    Default configuration nbRedundancy = 60.

Memory required can be calculated via:

(nbRedundancy / 8 + 1) \* nbRedundancy -\> MatrixM2B

\+ nbFrag \* 2 -\> FragNbMissingIndex

\+ nbFrag / 8 + 1 -\> matrixRow

\+ fragSize -\> matrixDataTemp

\+ (nbRedundancy / 8 + 1) \* 3 -\> dataTempVector, dataTempVector2 and S

For a default configuration this comes down to \~2 444 bytes of RAM.

In addition, some small allocations are made when xor'ing lines while applying
the redundant packets.

*Smart Delta library flash and RAM requirements*

Smart Delta library requires approximately 4K of MCU flash and 2,3K of RAM.

*Network requirements and implementation limitations*

In order to support FUOTA LoRaWAN network should support following features:

-   LoRaWAN multicast support

-   Time synchronization. Either support of MAC level DeviceTimeReq/Ans commands
    or support of application-level time synchronization. Actility FUOTA Service
    supports application-level time synchronization.

Current device code does not support Class B multicast session. This limitation
will be lifted in the future.

# Software installation and configuration

- Install STM32Cube IDE 1.12.0 or later:
<https://www.st.com/en/development-tools/stm32cubeide.html>

- Clone this git repository (`git@github.com:actility/nucleo-stm32wl55jc-fuota`
or <https://github.com/actility/nucleo-stm32wl55jc-fuota>)

- Start STM32CubeIDE application

- Close welcome screen tab (the `Information Center`) if any

- In `File` \> `Open Projects from File system`, make sure that 
`Search for nested projects` is enabled, press `Directory`, and select the
directory with cloned git repository. In the list with found projects unselect
the first project and select the following projects:

    - `<root_dir>\Projects\NUCLEO-WL55JC\Applications\LoRaWAN_FUOTA\STM32CubeIDE\1_Image_SECoreBin`
    - `<root_dir>\Projects\NUCLEO-WL55JC\Applications\LoRaWAN_FUOTA\STM32CubeIDE\1_Image_BFU`
    - `<root_dir>\Projects\NUCLEO-WL55JC\Applications\LoRaWAN_FUOTA\STM32CubeIDE\LoRaWAN_End_Node`

- In Project explorer window, right click on project name listed below and then
click the “Build” button. Do it for every project in order:

    - 1_Image_SECoreBin

    - 1_Image_BFU

    - LoRaWAN_End_Node

Following configuration options could be edited to adjust device code
configuration for specific requirements:

*Generic project configuration*

In order to configure generic project options:

-   Right-click on project LoRaWAN_End_Node

-   Select Properties \> C/C++ General \> Paths and Symbols

-   In Symbols tab, adapt the following symbols:

| **Symbol**                                      | **Description**                                                                                            | **Setting**                                                                                                                                                               |
|-------------------------------------------------|------------------------------------------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| ACTILITY_SMARTDELTA                             | Selects type firmware upgrade performed.                                  | In current project only SMARTDELTA = 1 is supported                                                                                                                                                        |

Leave all other symbols as is

*Device credentials configuration*

In order to customize the `devEUI`/`JoinEUI`/`AppKey` device parameters use
following procedure:

-    Edit
`<root_dir>\Projects\NUCLEO-STM32WL55JC\Applications\LoraWAN_FUOTA\LoRaWAN_End_Node\LoRaWAN\App\se-identity.h`

-   Customize these parameters: `STATIC_DEVICE_ADDRESS`,`LORAWAN_DEVICE_EUI`, `LORAWAN_JOIN_EUI`,
    `LORAWAN_NWK_KEY`
	
	Set STATIC_DEVICE_ADDRESS to 0 if you want automatic assignment of LORAWAN_DEVICE_EUI and LORAWAN_JOIN_EUI

*Fragmentation decoder configuration*

Fragmentation decoder configuration should be adjusted if you want to adapt
maximum size of firmware received for data rates you plan to use in specific RF
region. Refer to regional parameters LoRaWAN document to obtain maximum size of
the frame (data fragment size will be size of frame minus MAC header) in
selected RF regions with given data rates. Maximum data fragment size multiplied
by maximum number of fragments will define largest size of firmware image to be
received.

Adapt maximum number of redundancy fragments according to expected maximum
packet loss in the network and maximum number of the fragments expected.
Typically, it will be required to receive a little bit more redundancy packets
(from 2 to 10) than were lost to recover initial data file.

See “Fragmentation algorithm RAM overhead calculation” to calculate required RAM
size with selected fragmentation decoder configuration.

To customize fragmentation decoder parameters:

-   Edit
    `<root_dir>\Middlewares\Third_Party\LoRaWAN\LmHandler\Packages\frag_decoder_if.h`

| **Symbol**          | **Description**                                     | **Setting**                    |
|---------------------|-----------------------------------------------------|--------------------------------|
| FRAG_MAX_NB         | Maximum number of fragments which could be received | Default setting 1000 fragments |
| FRAG_MAX_SIZE       | Maximum fragment size                               | Default setting 222 bytes      |
| FRAG_MAX_REDUNDANCY | Maximum number of redundancy fragments              | Default setting 30 fragments   |

Above parameters works well if you are using DR5 in your multicast group. If your are using other data rates
especially DR0 or DR1 you will probably need to rise FRAG_MAX_NB and FRAG_MAX_REDUNDANCY to match typical size 
of the firware image or data file you transfer. This is especially true if during the FUOTA Campaigns you receive 
"Not enough memory" error in response to FraSessionReq command.

# Flashing firmware and first start

In order to flash compiled binary into device use following procedure:

-   Install STM32Cube Programmer 2.13.0:
    <https://www.st.com/en/development-tools/stm32cubeprog.html>

-   Start STM32Cube Programmer

    -   Click on `Open file` and select
        `<root_dir>\Projects\NUCLEO-STM32WL55JC\Applications\LoraWAN_FUOTA\LoRaWAN_End_Node\STM32CubeIDE\Binary\BFU_LoRaWAN_End_Node.bin`

        -   Click `Download`. At this point, the firmware is loaded onto the
            device

        -   Click `Disconnect`

In order to verify that device correctly operates after flashing you will need
to connect to the device virtual COM port. You should do it before flashing. On 
Windows, to obtain COM port number:

-   Press Win+X and select “Device manager”

-   In the section “Ports (COM and LPT)” look for COM port with description
    “STLink Virtual COM Port” and note its number in parenthesis, e.g. COM8.

-   Run your favorite terminal emulator, e.g. TeraTerm, and connect to serial
    port you obtain above (COM8) with following parameters: speed 115200, data
    bits 8, stop bits 1, parity NONE.

-   Check that device boots up correctly and you can see the following output in
    the terminal emulator window

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
= [SBOOT] RuntimeProtections: 0
= [SBOOT] System Security Check successfully passed. Starting...
= [FWIMG] Slot #0 @: 806a000 / Slot #1 @: 8014000 / Swap @: 80b4000

======================================================================
=              (C) COPYRIGHT 2017 STMicroelectronics                 =
=                                                                    =
=              Secure Boot and Secure Firmware Update                =
======================================================================

= [SBOOT] SECURE ENGINE INITIALIZATION SUCCESSFUL
= [SBOOT] STATE: CHECK STATUS ON RESET
	  INFO: A Reboot has been triggered by a Hardware reset!
	  Consecutive Boot on error counter reset 
	  Consecutive Boot on error counter = 0 
	  Consecutive Boot on error counter updated 
	  INFO: Last execution status before Reboot was:Executing Fw Image.
	  INFO: Last execution detected error was:No error. Success.
= [SBOOT] STATE: CHECK NEW FIRMWARE TO DOWNLOAD
= [SBOOT] STATE: CHECK USER FW STATUS
	  A valid FW is installed in the active slot - version: 1
= [SBOOT] STATE: VERIFY USER FW SIGNATURE
= [SBOOT] RuntimeProtections: 0
= [SBOOT] STATE: EXECUTE USER FIRMWARE
APP_VERSION= 01.03.04.00
MAC_VERSION= 04.04.02.00
HW_VERSION= 01.BA.35.01
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

# Using firmware with FUOTA server

In order to test firmware update you first will need to configure full firmware
update campaign. Please, refer to ThingPark FUOTA documentation available [here](https://www.actility.com/thingpark-documentation-portal/).  
During the campaign creation you will be asked for firmware file to download.
Use
`<root_dir>\Projects\NUCLEO-STM32WL55JC\Applications\LoraWAN_FUOTA\LoRaWAN_End_Node\STM32CubeIDE\Binary\LoRaWAN_End_Node.sfb`
in this case.

To created Smart Delta update binary, you will need to specify two files of
different firmware versions. Use following procedure to create necessary
binaries:

-   Build and load firmware image into the device as described in “Flashing
    firmware and first start”.

-   Use
    `<root_dir>\Projects\NUCLEO-STM32WL55JC\Applications\LoraWAN_FUOTA\LoRaWAN_End_Node\STM32CubeIDE\Binary\LoRaWAN_End_Node.sfb`
    to create “from” file as described in Section 3.13 of the User Guide.   
    During a file creation use application version specified in
    `<root_dir>\Projects\NUCLEO-STM32WL55JC\Applications\LoraWAN_FUOTA\LoRaWAN_End_Node\LoRaWAN\App\lora_app_version.h`
    when asked.

-   Change something in device source code and update version in lora_app_version.h file.
    It is recommended to update just `__APP_VERSION_SUB2` symbol. Increment it by
    one for example.

-   Build new firmware image. Use LoRaWAN_End_Node.sfb from this build to create “to”
    file in GUI. Use updated version number when asked.

-   Create Smart Delta patch as described in Section 3.14 of the User Guide. You
    will use this Smart Delta binary when configuring new Campaign.

-   Run firmware update campaign and verify that device firmware was indeed
    updated using APP_VERSION information in the device boot screen.

In order to start playing with LoRaWAN FUOTA send an email to
partners.fuota@actility.com and ask for a free account.

# Appendixes

# Appendix A. Smart Delta library operation and API description

## Smart Delta core overview

Patching core is written in attempt to be system-independent, although taking in
account the usage at devices with restricted resources. Smart Delta creation
core functions require significantly more resources and targeted to regular
computers.

User of Smart Delta core must provide patch image to core functions and store
result of patching in system-depended manner.

The data in a patch image is presented as a stream which may be processed in one
pass. There is a header with metadata at the beginning of patch image describing
a kind of the image and containing parameters required to apply patch.

There are 2 basic functions to apply patch. First the patching must be
initialized by calling fota_patch_stream_init(...). Then one should call
fota_patch_stream_append(...) sequentially for every byte in a patch stream.

The result of patching is returned in callback function fota_write_target(...).
As patching core is supplied in sources this callback is ‘statically registered’
– user must provide it in his sources. The callback provides resulting image by
1 byte per call and user must store these bytes to collect whole new image.

There may be any number of calls to fota_write_target(...) inside 1 call to
fota_patch_stream_append(...) – from 0 to new image size.

## Functions description

*Initialization*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
fota_patch_result_t fota_patch_stream_init (
fota_patch_stream_state_t *pstream, 
uint8_t *ppatch, 
size_t patch_filled,
uint8_t *pram, 
size_t ram_size,
uint8_t *porig, 
size_t orig_room, 
size_t *ppatch_header_size
);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

Initializes patching process, checks if patch may be applied.

Arguments:

| Argument                           | Description                                                                                                                                                     |
|------------------------------------|-----------------------------------------------------------------------------------------------------------------------------------------------------------------|
| fota_patch_stream_state_t *pstream | Patching process context. It is used internally and must persist during whole patching process.                                                                 |
| uint8_t *ppatch                    | Pointer to start of patch image OR its first part if whole image unavailable by now. This first part must be more then patch image header (17 bytes worst case) |
| size_t patch_filled                | Size of patch image OR its first part                                                                                                                           |
| uint8_t *pram                      | Pointer to decompression buffer in RAM. This buffer must persist during whole patching process.                                                                 |
| size_t ram_size                    | Size of decompression buffer                                                                                                                                    |
| uint8_t *porig                     | Pointer to old image (to be patched)                                                                                                                            |
| size_t orig_room                   | Must be equal to size of old image or its slot size if image size unknown. Used for checking if old image size correct for patching.                            |
| size_t *ppatch_header_size         | Returns actual patch image header size to inform of patch image data offset from patch image start (data are right after header).                               |

Return values:

| Value                                    | Description                                                                                      |
|------------------------------------------|--------------------------------------------------------------------------------------------------|
| fotaOk                                   | Success                                                                                          |
| fotaHeaderSignatureSmall fotaHeaderSmall | Size of first part of patch image (supplied in patch_filled) too small, provide more bytes       |
| fotaHeaderParsBad                        | Patch image header parameters unknown, abort patching                                            |
| fotaPatchOrigSmall                       | Old image area (supplied in orig_room) less then expected old image size, abort patching         |
| fotaPatchOrigHash                        | Old image incorrect (hash do not match expected), abort patching                                 |
| fotaHeaderSignatureUnsupported           | Patch kind unsupported, abort patching                                                           |
| fotaPackLevelBig                         | Patch image compression level too high, increase compression buffer (supplied in pram, ram_size) |

*Patching*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
fota_patch_result_t fota_patch_stream_append(
fota_patch_stream_state_t *pstream,                                              uint8_t byte
); 
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

After initialization must be called sequentially for bytes of patch image data
(which are at offset returned in \*ppatch_header_size from patch image start).
New image is ready when all bytes consumed.

Arguments:

| Argument                           | Description                                                                                     |
|------------------------------------|-------------------------------------------------------------------------------------------------|
| fota_patch_stream_state_t *pstream | Patching process context. It is used internally and must persist during whole patching process. |
| uint8_t byte                       | Next byte of patch image data                                                                   |

Return values:

| Value                                    | Description                                                   |
|------------------------------------------|---------------------------------------------------------------|
| fotaOk                                   | Success                                                       |
| fotaLzgMiss                              | Decompression failure, abort patching (patch image corrupted) |
| fotaBspatchOrigMiss                      | Patching failure, abort patching (patch image corrupted)      |
| fotaTargetOverflow fotaTargetWriteFailed | Translated from fota_write_target(...), abort patching        |

##### 

*Storing result*

~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
fota_patch_result_t fota_write_target(
uint8_t b
);
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~

User supplied callback to store new image. Called 0 or more times inside
fota_patch_stream_append(...), sequentially provides bytes of new image.
Depending on FLASH requirements it may write supplied byte directly to output
area or collect writable page in intermediate buffer.

Arguments:

| Argument     | Description                      |
|--------------|----------------------------------|
| uint8_t byte | Next byte of (patched) new image |

Return values:

| Value                 | Description                                                                           |
|-----------------------|---------------------------------------------------------------------------------------|
| fotaOk                | Success (byte stored)                                                                 |
| fotaTargetOverflow    | New image area overflowed, patching impossible                                        |
| fotaTargetWriteFailed | Byte write error, patching must be aborted (retries not implemented in patching core) |

# Appendix B. Porting considerations

Porting this FUOTA device code to another ST Microelectronics MCU requires prior evaluation of flash and RAM 
available on these platforms and flash sector size.

If ported to another platform, current configuration of the code requires splitting of
available flash into 3 regions: active slot, update slot and swap slot plus bootloader area.  
For example, with 256 KB of flash, actual firmware size could be not be larger than around 91 KBytes (space remaining from 
slots and swap is used for bootloader).  

RAM required by this code (without user applications or RTOS) consists of RAM required for Smart Delta (r) 
and RAM required for fragmentation decoder. RAM required for fragmentation decoder can be calculated
using guidelines in section "Fragmentation algorithm RAM overhead calculation". 2,3 KB of RAM is 
required for Smart Delta (r) with optimal compression. Consider this total RAM overhead when planning
integration. 

Current code is written with equally divided small size flash pages in mind. If an MCU considered for porting
has flash pages of non-equal size and especially of large size, it could require complete redesign of 
flash handling operations and potentially impact RAM requirements and flash organization.