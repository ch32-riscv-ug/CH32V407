/********************************** (C) COPYRIGHT *******************************
* File Name          : usb_driver.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb driver headfile.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __USB_DRIVER_H_
#define __USB_DRIVER_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
// include common headfile.
#include "usb_define.h"
#include "usb_config.h"

// include usb device driver headfile.
#include "device/usbd_driver_public.h"

// include usb host driver headfile.
#include "host/usbh_driver_public.h"

// include usb class driver headfile.
#include "class/hid/hid.h"
#include "class/cdc/cdc.h"
#include "class/cdc/cdc_rndis.h"
#include "class/msc/msc.h"
#include "class/uac/uac.h"
#include "class/uvc/uvc.h"

// include usb device class driver headfile.
#include "class/hub/hub.h"
#include "class/hid/hidd.h"
#include "class/cdc/cdcd.h"
#include "class/msc/mscd.h"
#include "class/uac/uacd.h"
#include "class/uvc/uvcd.h"

// include usb host class driver headfile.
#include "class/hub/hubh.h"

#ifdef __cplusplus
}
#endif

#endif
