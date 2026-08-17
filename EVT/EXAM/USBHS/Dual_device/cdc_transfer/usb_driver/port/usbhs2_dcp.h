/********************************** (C) COPYRIGHT *******************************
* File Name          : usbhs1_dcp.h
* Author             : WCH
* Version            : V1.2
* Date               : 2026/05/26
* Description        : Usb high speed device controller 2 port for ch32v407.
*********************************************************************************
* Copyright (c) 2026 Nanjing Qinheng Microelectronics Co., Ltd.
* Attention: This software (modified or not) and binary are used for
* microcontroller manufactured by Nanjing Qinheng Microelectronics.
*******************************************************************************/

#ifndef __USBHS2_DCP_H_
#define __USBHS2_DCP_H_

#ifdef __cplusplus
extern "C" {
#endif

/* @include */
#include "usb_driver.h"

/* @define */
#define USBHS2_DCP_MAX_ENDPOINTS        8
#define USBHS2_DCP_ENDP0_SIZE           64

/* @extern */
extern usbd_handle_t usbhs2d_handle;

/* @function declaration */
usb_rst_e usbhs2_dch_init(void);

#ifdef __cplusplus
}
#endif

#endif
