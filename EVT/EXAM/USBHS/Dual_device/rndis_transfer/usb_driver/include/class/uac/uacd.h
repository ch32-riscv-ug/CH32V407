/********************************** (C) COPYRIGHT *******************************
* File Name          : uacd.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Uac device class define.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __UACD_H_
#define __UACD_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include <stdint.h>

#include "usb_define.h"
#include "class/uac/uac.h"

/* @struct */
typedef struct
{
    uint8_t itf_num;

    usb_endp_t ep_iso;

    uint8_t alt_setting;
} uacd_streaming_itf_t;

#ifdef __cplusplus
}
#endif

#endif
